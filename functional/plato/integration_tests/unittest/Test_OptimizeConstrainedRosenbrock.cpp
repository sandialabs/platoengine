#include <gtest/gtest.h>

#include <ROL_Bounds.hpp>
#include <ROL_LineSearchStep.hpp>
#include <ROL_Problem.hpp>
#include <ROL_Ptr.hpp>
#include <ROL_RandomVector.hpp>
#include <ROL_StatusTest.hpp>
#include <ROL_StdVector.hpp>
#include <ROL_Stream.hpp>
#include <Teuchos_GlobalMPISession.hpp>
#include <iomanip>

#include "plato/core/Compose.hpp"
#include "plato/criteria/library/ConstraintAdapter.hpp"
#include "plato/criteria/library/TargetOffsetFunction.hpp"
#include "plato/integration_tests/utilities/DynamicVectorRosenbrockFunction.hpp"
#include "plato/integration_tests/utilities/DynamicVectorSumConstraintUtilities.hpp"
#include "plato/process_manager/extension/ROLUtilities.hpp"
#include "plato/test_utilities/SumConstraint.hpp"
#include "plato/third_party_integration/rol/ROLConstraint.hpp"
#include "plato/third_party_integration/rol/ROLObjectiveFunction.hpp"
#include "plato/third_party_integration/rol/ROLVectorConstraintFunction.hpp"
#include "plato/third_party_integration/rol/Utilities.hpp"

namespace plato::integration_tests::serial
{
namespace
{
using FunctionArgumentType = const linear_algebra::DynamicVector<double>&;

constexpr double kGoldXValue = 0.6187956185408358;
constexpr double kGoldYValue = 0.3812043814594129;
// ROL uses a different algorithm when solving Linear + Nonlinear vs Vector(Nonlinear) constraints
constexpr double kTolerance = 1e-6;

constexpr bool kPrintFlag = true;

[[nodiscard]] auto composed_target_function(const criteria::library::VectorFunction<FunctionArgumentType>& aFunction,
                                            const double aTarget)
{
    return core::compose(criteria::library::make_target_offset_function(aTarget), aFunction);
}

[[nodiscard]] ROL::ParameterList create_parameter_list()
{
    ROL::ParameterList tParlist;
    tParlist.sublist("Step").set("Type", "Augmented Lagrangian");
    const auto tOutputLevel = tParlist.sublist("General").get("Output Level", 1);
    tParlist.sublist("General").set("Output Level", tOutputLevel);
    tParlist.sublist("General").set("Inexact Hessian-Times-A-Vector", true);
    tParlist.sublist("General").sublist("Secant").set("Use as Hessian", true);
    return tParlist;
}

[[nodiscard]] auto to_dynamic_vector(const test_utilities::TwoDVector& aX) -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>{std::vector{aX(0), aX(1)}};
}

[[nodiscard]] auto make_sum_constraint_dynamic_vector_function(const test_utilities::SumConstraint& aSumConstraint)
{
    return core::make_function_with_first_derivative([aSumConstraint](const linear_algebra::DynamicVector<double>& x)
                                                     { return aSumConstraint.f(x[0], x[1]); },
                                                     [aSumConstraint](const linear_algebra::DynamicVector<double>& x)
                                                     { return to_dynamic_vector(aSumConstraint.df(x[0], x[1])); });
}

[[nodiscard]] auto create_line_constraint()
    -> std::unique_ptr<third_party_integration::rol::ROLVectorConstraintFunction>
{
    constexpr bool tLineLinear = true;
    constexpr double tSumConstraintTarget = 1.0;

    const auto tConstraintFunction = criteria::library::to_vector_function<FunctionArgumentType>(
        make_sum_constraint_dynamic_vector_function(test_utilities::SumConstraint{}));

    auto tConstraint = criteria::library::VectorConstraint<FunctionArgumentType>{
        .mName = "Line",
        .mConstraintFunction = composed_target_function(tConstraintFunction, tSumConstraintTarget),
        .mLinear = tLineLinear,
        .mConstraintType = criteria::library::ConstraintType::kLessThan};

    auto tROLConstraint = third_party_integration::rol::ROLVectorConstraintFunction{std::move(tConstraint)};
    return std::make_unique<third_party_integration::rol::ROLVectorConstraintFunction>(tROLConstraint);
}

[[nodiscard]] auto create_circle_constraint()
    -> std::unique_ptr<third_party_integration::rol::ROLVectorConstraintFunction>
{
    constexpr auto tCircleLinear = bool{false};
    constexpr auto tCircleConstraintTarget = double{1.0};
    constexpr auto tCenter = std::make_pair(0.0, 0.0);

    const auto tConstraintFunction = criteria::library::to_vector_function<FunctionArgumentType>(
        make_sum_constraint_dynamic_vector_function(test_utilities::SumConstraint{tCenter, 2}));

    auto tConstraint = criteria::library::VectorConstraint<FunctionArgumentType>{
        .mName = "Circle",
        .mConstraintFunction = composed_target_function(tConstraintFunction, tCircleConstraintTarget),
        .mLinear = tCircleLinear,
        .mConstraintType = criteria::library::ConstraintType::kLessThan};
    auto tROLConstraint = third_party_integration::rol::ROLVectorConstraintFunction{std::move(tConstraint)};
    return std::make_unique<third_party_integration::rol::ROLVectorConstraintFunction>(std::move(tROLConstraint));
}

[[nodiscard]] auto create_rol_bounded_rosenbrock_problem(ROL::Ptr<ROL::StdVector<double>>& aControls)
    -> std::unique_ptr<ROL::Problem<double>>
{
    auto tObjective = std::make_unique<third_party_integration::rol::ROLObjectiveFunction>(
        utilities::make_rosenbrock_dynamic_vector_function(test_utilities::Rosenbrock{}));

    auto tROLProblem =
        std::make_unique<ROL::Problem<double>>(ROL::Ptr<ROL::StdObjective<double>>(tObjective.release()), aControls);

    const auto tLower = std::vector{-1.0, -1.0};
    const auto tUpper = std::vector{1.0, 1.0};

    auto tBounds = third_party_integration::rol::create_rol_bound_constraint({tLower, tUpper});

    tROLProblem->addBoundConstraint(tBounds);
    return tROLProblem;
}

void finalize_rol_problem(ROL::Problem<double>& aROLProblem)
{
    constexpr bool tLumpConstraints = false;
    ROL::Ptr<std::ostream> tOutStream = ROL::makePtrFromRef(std::cout);
    aROLProblem.finalize(tLumpConstraints, kPrintFlag, *tOutStream);
}

void add_linear_constraint_rol_problem(
    ROL::Problem<double>& aROLProblem,
    std::unique_ptr<third_party_integration::rol::ROLVectorConstraintFunction>&& aConstraint)
{
    constexpr auto tDualVectorSize = 1U;
    auto tInequalityBoundConstraint =
        third_party_integration::rol::detail::create_less_than_inequality_bounds(tDualVectorSize);
    auto tMultipliers = ROL::makePtr<std::vector<double>>(tDualVectorSize, 0);
    auto tMultipliersPtr = ROL::makePtr<ROL::StdVector<double>>(tMultipliers);

    aROLProblem.addLinearConstraint(
        "Line", ROL::Ptr<ROL::StdConstraint<double>>(std::move(aConstraint).release()),
        third_party_integration::rol::make_rol_vector(criteria::library::make_dual_vector(tDualVectorSize)),
        tInequalityBoundConstraint, tMultipliersPtr, false);
}

template <typename ConstraintType>
void add_nonlinear_constraint_rol_problem(ROL::Problem<double>& aROLProblem,
                                          std::unique_ptr<ConstraintType>&& aConstraint,
                                          const unsigned int aDualSize)
{
    auto tInequalityBoundConstraint =
        third_party_integration::rol::detail::create_less_than_inequality_bounds(aDualSize);
    auto tMultipliers = ROL::makePtr<std::vector<double>>(aDualSize, 0);
    auto tMultipliersPtr = ROL::makePtr<ROL::StdVector<double>>(tMultipliers);

    aROLProblem.addConstraint(
        "Sum", ROL::Ptr<ROL::StdConstraint<double>>(std::move(aConstraint).release()),
        third_party_integration::rol::make_rol_vector(criteria::library::make_dual_vector(aDualSize)),
        tInequalityBoundConstraint, tMultipliersPtr, false);
}

[[nodiscard]] auto create_rol_vector_constraint_combination_line_and_circle()
{
    constexpr bool tIsLinear = false;
    constexpr double tTarget = 1.0;

    auto tConstraint = criteria::library::VectorConstraint<const linear_algebra::DynamicVector<double>&>{
        .mName = "Circle and line",
        .mConstraintFunction = composed_target_function(utilities::make_line_and_circle_jacobian_function(), tTarget),
        .mLinear = tIsLinear,
        .mConstraintType = criteria::library::ConstraintType::kLessThan};

    return third_party_integration::rol::ROLVectorConstraintFunction{std::move(tConstraint)};
}

[[nodiscard]] auto rol_constrained_rosenbrock_problem_with_two_scalar_constraints(
    ROL::Ptr<ROL::StdVector<double>>& aControls) -> std::unique_ptr<ROL::Problem<double>>
{
    auto tROLProblem = create_rol_bounded_rosenbrock_problem(aControls);
    add_linear_constraint_rol_problem(*tROLProblem, create_line_constraint());
    constexpr auto tDualSize = 1U;
    add_nonlinear_constraint_rol_problem(*tROLProblem, create_circle_constraint(), tDualSize);

    finalize_rol_problem(*tROLProblem);
    return tROLProblem;
}

[[nodiscard]] auto rol_constrained_rosenbrock_problem_with_one_vector_constraint(
    ROL::Ptr<ROL::StdVector<double>>& aControls) -> std::unique_ptr<ROL::Problem<double>>
{
    auto tROLProblem = create_rol_bounded_rosenbrock_problem(aControls);
    auto tConstraint = std::make_unique<third_party_integration::rol::ROLVectorConstraintFunction>(
        create_rol_vector_constraint_combination_line_and_circle());
    constexpr auto tDualSize = 2U;
    add_nonlinear_constraint_rol_problem(*tROLProblem, std::move(tConstraint), tDualSize);
    finalize_rol_problem(*tROLProblem);
    return tROLProblem;
}

struct SolutionForStream
{
    std::reference_wrapper<const linear_algebra::DynamicVector<double>> mSolution;
};

template <typename OutStream>
auto operator<<(OutStream& aOutStream, const SolutionForStream& aSolution) -> OutStream&
{
    const auto& tVector = aSolution.mSolution.get();
    aOutStream << std::setprecision(16) << "tSolution[0]" << tVector[0] << "\n";
    aOutStream << "tSolution[1]" << tVector[1] << "\n";
    aOutStream << "tSolution[0]+tSolution[1] = " << tVector[0] + tVector[1] << "\n";
    aOutStream << "tSolution[0]^2+tSolution[1]^2  = " << tVector[0] * tVector[0] + tVector[1] * tVector[1] << "\n";
    aOutStream << "Rosenbrock: " << test_utilities::Rosenbrock{}.f(tVector[0], tVector[1]) << "\n";
    return aOutStream;
}

void print_and_test_solution(const linear_algebra::DynamicVector<double>& aSolution)
{
    const auto tSolutionForStream = SolutionForStream{std::ref(aSolution)};
    ASSERT_EQ(aSolution.size(), 2u) << tSolutionForStream;
    EXPECT_NEAR(aSolution[0], kGoldXValue, kTolerance) << tSolutionForStream;
    EXPECT_NEAR(aSolution[1], kGoldYValue, kTolerance) << tSolutionForStream;
}

void check_rol_optimization(std::unique_ptr<ROL::Problem<double>>&& aROLProblem,
                            const ROL::StdVector<double>& aControls)
{
    auto tROLInputs = create_parameter_list();
    auto tROLProblem = ROL::Ptr<ROL::Problem<double>>(aROLProblem.release());

    auto tROLSolver = process_manager::extension::make_rol_solver(tROLInputs, tROLProblem);
    ROL::Ptr<std::ostream> tOutStream = ROL::makePtrFromRef(std::cout);
    tROLSolver.solve(*tOutStream);
    const auto tSolution = third_party_integration::rol::to_dynamic_vector(aControls);
    print_and_test_solution(tSolution);
}

}  // namespace

TEST(Optimize, RosenbrockWithConstraintsROLTwoScalarConstraints)
{
    auto tControl = ROL::makePtr<ROL::StdVector<double>>(2, 0.0);
    check_rol_optimization(rol_constrained_rosenbrock_problem_with_two_scalar_constraints(tControl), *tControl);
}

TEST(Optimize, RosenbrockWithConstraintsROLOneVectorConstraint)
{
    auto tControl = ROL::makePtr<ROL::StdVector<double>>(2, 0.0);
    check_rol_optimization(rol_constrained_rosenbrock_problem_with_one_vector_constraint(tControl), *tControl);
}

}  // namespace plato::integration_tests::serial
