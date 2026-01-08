#include "plato/process_manager/extension/ROLUtilities.hpp"

#include <ROL_Bounds.hpp>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/core/Compose.hpp"
#include "plato/criteria/library/ConstraintAdapter.hpp"
#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/output/OutputManager.hpp"
#include "plato/process_manager/extension/ConstraintCompositionUtility.hpp"
#include "plato/process_manager/extension/ROLOptimization.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/services/SystemLogger.hpp"
#include "plato/third_party_integration/rol/ROLConstraint.hpp"
#include "plato/third_party_integration/rol/Utilities.hpp"
#include "plato/utilities/BoostOptionalToStdOptional.hpp"

namespace plato::process_manager::extension
{
namespace
{
const auto kConstraintTypeConversion =
    std::map<criteria::library::ConstraintType, third_party_integration::rol::ConstraintType>{
        {criteria::library::ConstraintType::kEqualTo, third_party_integration::rol::ConstraintType::kEqualTo},
        {criteria::library::ConstraintType::kGreaterThan, third_party_integration::rol::ConstraintType::kGreaterThan},
        {criteria::library::ConstraintType::kLessThan, third_party_integration::rol::ConstraintType::kLessThan}};

[[nodiscard]] auto load_file_or_use_default_parameters(const input_parser::rol_optimization& aOptimizationParameters)
    -> third_party_integration::rol::OptimizationParameters
{
    if (aOptimizationParameters.input_file_name)
    {
        return third_party_integration::rol::OptimizationParameters(
            aOptimizationParameters.input_file_name.value().mToken);
    }
    return third_party_integration::rol::OptimizationParameters();
}

void apply_verbose_output(const input_parser::rol_optimization& aOptimizationParameters,
                          third_party_integration::rol::OptimizationParameters& aParameters)
{
    if (aOptimizationParameters.verbose_output.value_or(false))
    {
        aParameters.verbose();
    }
}

void apply_approximate_hessian(const input_parser::rol_optimization& aOptimizationParameters,
                               third_party_integration::rol::OptimizationParameters& aParameters)
{
    if (aOptimizationParameters.approximate_hessian.value_or(false))
    {
        aParameters.approximateHessian();
    }
}

void write_parameters(const input_parser::rol_optimization& aOptimizationParameters,
                      third_party_integration::rol::OptimizationParameters& aParameters)
{
    if (aOptimizationParameters.export_settings_file_name)
    {
        aParameters.writeParameters(aOptimizationParameters.export_settings_file_name.value().mToken);
    }
}

}  // namespace

auto make_rol_objective(const library::ProcessManagerData& aProblem, output::OutputManager aOutputManager)
    -> std::unique_ptr<plato::third_party_integration::rol::ROLObjectiveFunction>
{
    return std::make_unique<plato::third_party_integration::rol::ROLObjectiveFunction>(
        compose(aProblem.mObjective, aProblem.mGeometry.mCompute), std::move(aOutputManager));
}

auto make_rol_constraints(const library::ProcessManagerData& aProblem)
    -> std::vector<third_party_integration::rol::ROLConstraint>
{
    std::vector<third_party_integration::rol::ROLConstraint> tROLConstraints;
    std::transform(
        aProblem.mConstraints.cbegin(), aProblem.mConstraints.cend(), std::back_inserter(tROLConstraints),
        [&aProblem](const auto& aConstraintData)
        {
            const auto tComposedVectorConstraint =
                compose_geometry_with_vector_constraint(aConstraintData, aProblem.mGeometry);

            const auto tConstraintSize =
                tComposedVectorConstraint.mConstraintFunction
                    .template evaluate<core::evaluation::kFunction>(aProblem.mGeometry.mInitialGuess)
                    .size();

            return third_party_integration::rol::ROLConstraint{
                aConstraintData.mName, tConstraintSize, aConstraintData.mLinear,
                kConstraintTypeConversion.at(aConstraintData.mConstraintType),
                std::make_unique<third_party_integration::rol::ROLVectorConstraintFunction>(
                    tComposedVectorConstraint.mConstraintFunction)};
        });
    return tROLConstraints;
}

auto make_rol_problem(const library::ProcessManagerData& aProblem,
                      const std::string_view aProcessManagerName,
                      output::OutputManager aOutputManager)
    -> std::pair<ROL::Ptr<ROL::Problem<double>>, ROL::Ptr<ROL::StdVector<double>>>
{
    return make_rol_problem(aProblem, aProcessManagerName,
                            ROL::Ptr<plato::third_party_integration::rol::ROLObjectiveFunction>(
                                make_rol_objective(aProblem, std::move(aOutputManager)).release()));
}

auto make_rol_problem(const library::ProcessManagerData& aProblem,
                      const std::string_view aProcessManagerName,
                      const ROL::Ptr<ROL::StdObjective<double>>& aROLObjective)
    -> std::pair<ROL::Ptr<ROL::Problem<double>>, ROL::Ptr<ROL::StdVector<double>>>
{
    namespace tpir = third_party_integration::rol;
    auto tControls = tpir::make_rol_vector(aProblem.mGeometry.mInitialGuess);
    auto tROLProblem = ROL::makePtr<ROL::Problem<double>>(aROLObjective, tControls);
    tROLProblem->addBoundConstraint(tpir::create_rol_bound_constraint(aProblem.mGeometry.mBounds));
    for (auto&& tConstraint : make_rol_constraints(aProblem))
    {
        add_constraint_to_problem(*tROLProblem, std::move(tConstraint));
    }

    ///@todo Determine how ROL lumps constraints - should this only be false if they are all linear constraints?
    constexpr bool tLumpConstraints = false;
    constexpr bool tPrintToStream = true;
    auto tStream = std::stringstream{};
    tStream << "\n";
    tROLProblem->finalize(tLumpConstraints, tPrintToStream, tStream);

    auto tLogger = services::component_logger(components::ComponentType::kProcessManager, aProcessManagerName);
    tLogger.logInfo(tStream.str());

    return {tROLProblem, tControls};
}

auto make_rol_solver(Teuchos::ParameterList& aROLOptions, const ROL::Ptr<ROL::Problem<double>>& aROLProblem)
    -> ROL::Solver<double>
{
    return ROL::Solver<double>{aROLProblem, aROLOptions};
}

auto make_optimization_parameters(const library::ValidatedProcessManagerInput& aOptimizationParameters)
    -> third_party_integration::rol::OptimizationParameters
{
    const auto& aROLParameters =
        input_validation::get_input_block<input_parser::rol_optimization>(aOptimizationParameters);

    auto tParameters = load_file_or_use_default_parameters(aROLParameters);

    tParameters.maximumIterations(utilities::to_std_optional(aROLParameters.max_iterations));
    tParameters.gradientTolerance(utilities::to_std_optional(aROLParameters.gradient_tolerance));
    tParameters.stepTolerance(utilities::to_std_optional(aROLParameters.step_tolerance));
    tParameters.initialSearchRadius(utilities::to_std_optional(aROLParameters.initial_search_radius));
    tParameters.writeOutputHistory(utilities::to_std_optional(aROLParameters.output_design_history));

    apply_verbose_output(aROLParameters, tParameters);
    apply_approximate_hessian(aROLParameters, tParameters);
    write_parameters(aROLParameters, tParameters);

    return tParameters;
}

}  // namespace plato::process_manager::extension
