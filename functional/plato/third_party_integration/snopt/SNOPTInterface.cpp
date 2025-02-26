#include "plato/third_party_integration/snopt/SNOPTInterface.hpp"

#include <boost/numeric/conversion/cast.hpp>
#include <snoptProblem.hpp>

#include "plato/geometry/library/OutputManager.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/third_party_integration/snopt/DataSingleton.hpp"
#include "plato/third_party_integration/snopt/ObjectiveConstraintArrayView.hpp"
#include "plato/third_party_integration/snopt/SNOPTConstraints.hpp"
#include "plato/third_party_integration/snopt/SNOPTUtilities.hpp"

namespace plato::third_party_integration::snopt
{

namespace
{
constexpr auto kObjectiveRow = int{0};
constexpr auto kNumberOfObjectives = std::size_t{1};
constexpr auto kUseDerivatives = int{1};
constexpr auto kUseSummaryFile = int{1};
constexpr auto kColdStart = int{0};
constexpr auto kObjectiveValueAddition = double{0.0};
constexpr auto kNoSpecialBoundsInformation = int{0};
constexpr auto kDerivativeOptionName = std::string_view{"Derivative option"};

[[maybe_unused]] constexpr auto kSuccessfulSpecFileRead = int{101};
[[maybe_unused]] constexpr auto kSuccessfulParameterSet = int{0};
constexpr auto kMinutesToSeconds = int{60};

struct SNOPTTag
{
};

struct SNOPTSizes
{
    IndexType mNumberOfObjectivesAndConstraints;
    IndexType mNumberOfDesignVariablesAsInt;
    IndexType mNumberOfLinearConstraintJacobianEntries;
    IndexType mNumberOfNonlinearConstraintJacobianEntries;
};

struct SNOPTSolverDetail
{
    std::vector<double> mLowerDesignVariableBounds;
    std::vector<double> mUpperDesignVariableBounds;
    std::vector<IndexType> mSolutionBoundsInformation;
    std::vector<double> mBoundConstraintDualVariables;
    std::vector<double> mObjectiveAndConstraintValues;
    std::vector<IndexType> mObjectiveAndConstraintBoundsInformation;
    std::vector<double> mObjectiveAndConstraintDualVariables;

    int mNumberOfSuperBasicVariables = 0;
    int mNumberOfInfeasibleConstraints = 0;
    double mSumOfConstraintViolations = 0.0;
};

void evaluation_callback(int * /*Status*/,
                         int *aNumberOfDesignVariables,
                         double aDesignVariableArray[],
                         int *aEvaluateFunctions,                 // NOLINT
                         int *aNumberOfObjectivesAndConstraints,  // NOLINT
                         double aObjectiveAndConstraintValues[],
                         int *aEvaluateGradients,
                         int * /*neG*/,
                         double aObjectiveAndConstraintGradients[],
                         char * /*cu*/,
                         int * /*lencu*/,
                         int * /*iu[]*/,
                         int * /*leniu*/,
                         double * /*ru[]*/,
                         int * /*lenru*/)
{
    const auto tNumberOfDesignVariables = *aNumberOfDesignVariables;
    const auto tDesignVariables = linear_algebra::DynamicVector<double>{
        std::vector<double>(aDesignVariableArray, aDesignVariableArray + tNumberOfDesignVariables)};

    if (*aEvaluateFunctions > 0)
    {
        auto tObjectiveAndConstraintsView = ObjectiveConstraintArrayView<double>{
            aObjectiveAndConstraintValues,
            ConstraintSizeType{boost::numeric_cast<unsigned long>(*aNumberOfObjectivesAndConstraints)}};
        evaluateObjective<SNOPTTag>(tDesignVariables, tObjectiveAndConstraintsView);
        evaluateConstraints<SNOPTTag>(tDesignVariables, tObjectiveAndConstraintsView);
    }
    if (*aEvaluateGradients > 0)
    {
        auto tObjectiveAndConstraintGradientsView = ObjectiveConstraintGradientArrayView<double>{
            aObjectiveAndConstraintGradients,
            ConstraintSizeType{boost::numeric_cast<unsigned long>(*aNumberOfObjectivesAndConstraints)},
            DesignVariableSizeType{boost::numeric_cast<unsigned long>(tNumberOfDesignVariables)}};
        evaluateObjectiveGradient<SNOPTTag>(tDesignVariables, tObjectiveAndConstraintGradientsView);
        evaluateConstraintGradient<SNOPTTag>(tDesignVariables, tObjectiveAndConstraintGradientsView);
    }
}

[[nodiscard]] auto linear_constraint_row_offset(const SNOPTConstraints &aConstraints) -> SparsityPattern<IndexType>::Row
{
    return SparsityPattern<IndexType>::Row{
        boost::numeric_cast<IndexType>(aConstraints.numberOfNonlinearConstraints() + kNumberOfObjectives)};
}

[[nodiscard]] auto number_of_rows_for_g_matrix(const SNOPTConstraints &aConstraints) -> SparsityPattern<IndexType>::Row
{
    return SparsityPattern<IndexType>::Row{
        boost::numeric_cast<IndexType>(aConstraints.numberOfNonlinearConstraints() + kNumberOfObjectives)};
}

[[nodiscard]] auto number_of_columns_for_g_matrix(const std::size_t aNumberOfDesignVariables)
    -> SparsityPattern<IndexType>::Column
{
    return SparsityPattern<IndexType>::Column{boost::numeric_cast<IndexType>(aNumberOfDesignVariables)};
}

[[nodiscard]] auto nonlinear_constraints(const SNOPTConstraints &aSNOPTConstraints) -> ConstraintVectorType
{
    auto tNonlinearConstraints = ConstraintVectorType{};
    tNonlinearConstraints.reserve(aSNOPTConstraints.numberOfNonlinearConstraints());
    std::copy(aSNOPTConstraints.nonlinearConstraintsBegin(), aSNOPTConstraints.nonlinearConstraintsEnd(),
              std::back_inserter(tNonlinearConstraints));
    return tNonlinearConstraints;
}

[[nodiscard]] auto make_snopt_sizes(const SNOPTConstraints &aSNOPTConstraints,
                                    const std::size_t aNumberOfDesignVariables,
                                    const SparseMatrixBuilder<IndexType, double> &aLinearConstraintJacobian,
                                    const SparsityPattern<IndexType> &aNonlinearConstraintSparsityPattern) -> SNOPTSizes
{
    return {
        /*.mNumberOfObjectivesAndConstraints=*/boost::numeric_cast<IndexType>(kNumberOfObjectives +
                                                                              aSNOPTConstraints.constraints().size()),
        /*.mNumberOfDesignVariablesAsInt=*/boost::numeric_cast<IndexType>(aNumberOfDesignVariables),
        /*.mNumberOfLinearConstraintJacobianEntries=*/boost::numeric_cast<IndexType>(aLinearConstraintJacobian.size()),
        /*.mNumberOfNonlinearConstraintJacobianEntries=*/
        boost::numeric_cast<IndexType>(aNonlinearConstraintSparsityPattern.size())};
}

[[nodiscard]] auto make_snopt_solver_detail(const SNOPTSizes &aSNOPTSizes, const SNOPTBounds &aBoundConstraints)
    -> SNOPTSolverDetail
{
    return {
        /*.mLowerDesignVariableBounds=*/aBoundConstraints.first,
        /*.mUpperDesignVariableBounds=*/aBoundConstraints.second,
        /*.mSolutionBoundsInformation=*/
        std::vector<IndexType>(aSNOPTSizes.mNumberOfDesignVariablesAsInt, kNoSpecialBoundsInformation),
        /*.mBoundConstraintDualVariables=*/std::vector<double>(aSNOPTSizes.mNumberOfDesignVariablesAsInt),
        /*.mObjectiveAndConstraintValues=*/std::vector<double>(aSNOPTSizes.mNumberOfObjectivesAndConstraints),
        /*.mObjectiveAndConstraintBoundsInformation=*/
        std::vector<IndexType>(aSNOPTSizes.mNumberOfObjectivesAndConstraints, kNoSpecialBoundsInformation),
        /*.mObjectiveAndConstraintDualVariables=*/std::vector<double>(aSNOPTSizes.mNumberOfObjectivesAndConstraints),
    };
}

void shut_down_snopt()
{
    DataSingleton<ObjectiveType, SNOPTTag>::instance().reset();
    DataSingleton<ConstraintVectorType, SNOPTTag>::instance().reset();
    DataSingleton<geometry::library::OutputManager, SNOPTTag>::instance().reset();
}

}  // namespace

auto run_snopt_problem(const std::vector<double> &aInitialGuess,
                       const SNOPTBounds &aBoundConstraints,
                       ObjectiveType &&aObjective,
                       InterfaceConstraintVectorType &&aConstraints,
                       geometry::library::OutputManager &&aOutputManager,
                       const std::filesystem::path &aLogFilePath,
                       const SNOPTOptions &aOptions) -> std::vector<double>
{
    assert(aInitialGuess.size() == aBoundConstraints.first.size());
    assert(aInitialGuess.size() == aBoundConstraints.second.size());

    auto tSNOPTConstraints = SNOPTConstraints{std::move(aConstraints), aInitialGuess.size()};

    auto tObjectiveAndConstraintBounds = constraint_bounds_with_unbounded_objective(tSNOPTConstraints);

    const auto tNumberOfDesignVariables = aInitialGuess.size();

    auto tLinearConstraintJacobian =
        linear_constraint_jacobian_sparse_matrix(tSNOPTConstraints, tNumberOfDesignVariables);
    tLinearConstraintJacobian.sparsityPattern().offsetRows(linear_constraint_row_offset(tSNOPTConstraints));

    auto tNonlinearConstraintSparsityPattern =
        dense_matrix_sparsity_pattern(number_of_rows_for_g_matrix(tSNOPTConstraints),
                                      number_of_columns_for_g_matrix(tNumberOfDesignVariables), kObjectiveRow);

    const auto tSNOPTSizes = make_snopt_sizes(tSNOPTConstraints, tNumberOfDesignVariables, tLinearConstraintJacobian,
                                              tNonlinearConstraintSparsityPattern);

    // Variables for SNOPT interface, can't be const because SNOPT requires non-const pointers for all arguments
    auto tSNOPTSolverDetail = make_snopt_solver_detail(tSNOPTSizes, aBoundConstraints);

    DataSingleton<ObjectiveType, SNOPTTag>::instance().data() = std::move(aObjective);
    DataSingleton<ConstraintVectorType, SNOPTTag>::instance().data() = nonlinear_constraints(tSNOPTConstraints);
    DataSingleton<geometry::library::OutputManager, SNOPTTag>::instance().data() = std::move(aOutputManager);

    auto tSolution = aInitialGuess;

    auto tProblem = snoptProblemA{};
    tProblem.initialize(aLogFilePath.c_str(), kUseSummaryFile);

    detail::apply_options(tProblem, aOptions);
    tProblem.solve(
        kColdStart, tSNOPTSizes.mNumberOfObjectivesAndConstraints, tSNOPTSizes.mNumberOfDesignVariablesAsInt,
        kObjectiveValueAddition, kObjectiveRow, evaluation_callback, tLinearConstraintJacobian.rowData().get(),
        tLinearConstraintJacobian.columnData().get(), tLinearConstraintJacobian.entryData().get(),
        tSNOPTSizes.mNumberOfLinearConstraintJacobianEntries, tNonlinearConstraintSparsityPattern.rowData().get(),
        tNonlinearConstraintSparsityPattern.columnData().get(), tSNOPTSizes.mNumberOfNonlinearConstraintJacobianEntries,
        tSNOPTSolverDetail.mLowerDesignVariableBounds.data(), tSNOPTSolverDetail.mUpperDesignVariableBounds.data(),
        tObjectiveAndConstraintBounds.first.data(), tObjectiveAndConstraintBounds.second.data(), tSolution.data(),
        tSNOPTSolverDetail.mSolutionBoundsInformation.data(), tSNOPTSolverDetail.mBoundConstraintDualVariables.data(),
        tSNOPTSolverDetail.mObjectiveAndConstraintValues.data(),
        tSNOPTSolverDetail.mObjectiveAndConstraintBoundsInformation.data(),
        tSNOPTSolverDetail.mObjectiveAndConstraintDualVariables.data(), tSNOPTSolverDetail.mNumberOfSuperBasicVariables,
        tSNOPTSolverDetail.mNumberOfInfeasibleConstraints, tSNOPTSolverDetail.mSumOfConstraintViolations);

    auto &tOutputSingleton = DataSingleton<geometry::library::OutputManager, SNOPTTag>::instance();
    assert(tOutputSingleton.hasData());
    tOutputSingleton.data()->output(linear_algebra::DynamicVector<double>{tSolution});

    shut_down_snopt();

    return tSolution;
}

namespace detail
{

void apply_options(snoptProblemA &aProblem, const SNOPTOptions &aOptions)
{
    [[maybe_unused]] const int tErrorTally = aProblem.setIntParameter(kDerivativeOptionName.data(), kUseDerivatives);
    assert(tErrorTally == 0);

    if (aOptions.mFilePath)
    {
        [[maybe_unused]] const int tReturnCode = aProblem.setSpecsFile(std::string(aOptions.mFilePath.value()).c_str());
        assert(tReturnCode == kSuccessfulSpecFileRead);
    }
    if (aOptions.mTimeLimitInMinutes)
    {
        [[maybe_unused]] const int tReturnCode = aProblem.setIntParameter(
            snopt::kTimeLimitName.data(),
            boost::numeric_cast<int>(aOptions.mTimeLimitInMinutes.value() * kMinutesToSeconds));
        assert(tReturnCode == kSuccessfulParameterSet);
    }
    if (aOptions.mMajorIterationLimit)
    {
        [[maybe_unused]] const int tReturnCode = aProblem.setIntParameter(
            snopt::kMajorIterationLimitName.data(), boost::numeric_cast<int>(aOptions.mMajorIterationLimit.value()));
        assert(tReturnCode == kSuccessfulParameterSet);
    }
}
}  // namespace detail

}  // namespace plato::third_party_integration::snopt
