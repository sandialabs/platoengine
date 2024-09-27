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

#include "plato/integration_tests/utilities/DynamicVectorRosenbrockFunction.hpp"
#include "plato/integration_tests/utilities/DynamicVectorSumConstraintUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/test_utilities/SumConstraint.hpp"
#include "plato/third_party_integration/rol/OptimizerFactory.hpp"
#include "plato/third_party_integration/rol/ROLConstraint.hpp"
#include "plato/third_party_integration/rol/ROLConstraintFunction.hpp"
#include "plato/third_party_integration/rol/ROLObjectiveFunction.hpp"
#include "plato/third_party_integration/rol/Utilities.hpp"

namespace plato::integration_tests::serial
{
namespace
{
constexpr double kGoldXValue = 0.6187956185408358;
constexpr double kGoldYValue = 0.3812043814594129;
// ROL uses a different algorithm when solving Linear + Nonlinear vs Vector(Nonlinear) constraints
constexpr double kTolerance = 1e-6;

constexpr bool kPrintFlag = true;

[[nodiscard]] ROL::ParameterList create_parameter_list()
{
    ROL::ParameterList tParlist;
    tParlist.sublist("Step").set("Type", "Augmented Lagrangian");
    int outputLevel = tParlist.sublist("General").get("Output Level", 1);
    tParlist.sublist("General").set("Output Level", outputLevel);
    return tParlist;
}

[[nodiscard]] auto to_dynamic_vector(const test_utilities::TwoDVector& aX) -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>{std::vector{aX(0), aX(1)}};
}

[[nodiscard]] auto make_sum_constraint_dynamic_vector_function(const test_utilities::SumConstraint& aSumConstraint)
{
    return core::make_function([capture = aSumConstraint](const linear_algebra::DynamicVector<double>& x)
                               { return capture.f(x[0], x[1]); },
                               [capture = aSumConstraint](const linear_algebra::DynamicVector<double>& x)
                               { return to_dynamic_vector(capture.df(x[0], x[1])); });
}

std::unique_ptr<third_party_integration::rol::ROLConstraintFunction> create_line_constraint()
{
    constexpr bool tLineLinear = true;
    constexpr double tSumConstraintTarget = 1;

    criteria::library::Constraint<const linear_algebra::DynamicVector<double>&> tConstraint{
        "Line", make_sum_constraint_dynamic_vector_function(test_utilities::SumConstraint{}), tSumConstraintTarget,
        tLineLinear, criteria::library::ConstraintType::kLessThan};

    auto tROLConstraint = third_party_integration::rol::ROLConstraintFunction{std::move(tConstraint)};
    return std::make_unique<third_party_integration::rol::ROLConstraintFunction>(tROLConstraint);
}

std::unique_ptr<third_party_integration::rol::ROLConstraintFunction> create_circle_constraint()
{
    constexpr bool tCircleLinear = false;
    constexpr double tCircleConstraintTarget = 1;
    constexpr std::pair<double, double> tCenter{0, 0};

    criteria::library::Constraint<const linear_algebra::DynamicVector<double>&> tConstraint{
        "Circle", make_sum_constraint_dynamic_vector_function(test_utilities::SumConstraint{tCenter, 2}),
        tCircleConstraintTarget, tCircleLinear, criteria::library::ConstraintType::kLessThan};
    auto tROLConstraint = third_party_integration::rol::ROLConstraintFunction{std::move(tConstraint)};
    return std::make_unique<third_party_integration::rol::ROLConstraintFunction>(tROLConstraint);
}

std::unique_ptr<ROL::Problem<double>> create_rol_bounded_rosenbrock_problem(ROL::Ptr<ROL::StdVector<double>>& aControls)
{
    auto tObjective = std::make_unique<third_party_integration::rol::ROLObjectiveFunction>(
        utilities::make_rosenbrock_dynamic_vector_function(test_utilities::Rosenbrock{}));

    auto tROLProblem =
        std::make_unique<ROL::Problem<double>>(ROL::Ptr<ROL::StdObjective<double>>(tObjective.release()), aControls);

    const std::vector<double> tLower{-1, -1};
    const std::vector<double> tUpper{1, 1};

    auto tBounds = third_party_integration::rol::create_rol_bound_constraint({tLower, tUpper});

    tROLProblem->addBoundConstraint(tBounds);
    return tROLProblem;
}

void finalize_rol_problem(std::unique_ptr<ROL::Problem<double>>& aROLProblem)
{
    constexpr bool tLumpConstraints = false;
    ROL::Ptr<std::ostream> tOutStream = ROL::makePtrFromRef(std::cout);
    aROLProblem->finalize(tLumpConstraints, kPrintFlag, *tOutStream);
}

void add_linear_constraint_rol_problem(
    std::unique_ptr<ROL::Problem<double>>& aROLProblem,
    std::unique_ptr<third_party_integration::rol::ROLConstraintFunction>& aConstraint)
{
    auto tInequalityBoundConstraint = third_party_integration::rol::detail::create_less_than_inequality_bounds(1);
    auto tMultipliers = ROL::makePtr<std::vector<double>>(1, 0);
    auto tMultipliersPtr = ROL::makePtr<ROL::StdVector<double>>(tMultipliers);
    constexpr auto tDualVectorSize = 1U;

    aROLProblem->addLinearConstraint(
        "Line", ROL::Ptr<ROL::StdConstraint<double>>(aConstraint.release()),
        third_party_integration::rol::make_rol_vector(criteria::library::make_dual_vector(tDualVectorSize)),
        tInequalityBoundConstraint, tMultipliersPtr, false);
}

template <typename ConstraintType>
void add_nonlinear_constraint_rol_problem(std::unique_ptr<ROL::Problem<double>>& aROLProblem,
                                          std::unique_ptr<ConstraintType>& aConstraint,
                                          const unsigned int aDualSize)
{
    auto tInequalityBoundConstraint =
        third_party_integration::rol::detail::create_less_than_inequality_bounds(aDualSize);
    auto tMultipliers = ROL::makePtr<std::vector<double>>(aDualSize, 0);
    auto tMultipliersPtr = ROL::makePtr<ROL::StdVector<double>>(tMultipliers);

    aROLProblem->addConstraint(
        "Sum", ROL::Ptr<ROL::StdConstraint<double>>(aConstraint.release()),
        third_party_integration::rol::make_rol_vector(criteria::library::make_dual_vector(aDualSize)),
        tInequalityBoundConstraint, tMultipliersPtr, false);
}

auto create_rol_vector_constraint_combination_line_and_circle()
{
    constexpr bool tIsLinear = false;
    constexpr double tTarget = 1;

    const auto tJacobianFunction = utilities::make_line_and_circle_jacobian_function();
    const auto tAdjointJacobianFunction = utilities::make_line_and_circle_adjoint_jacobian_function();

    criteria::library::VectorConstraint<const linear_algebra::DynamicVector<double>&> tConstraint{
        "Circle and line", tJacobianFunction, tAdjointJacobianFunction,
        tTarget,           tIsLinear,         criteria::library::ConstraintType::kLessThan};

    return third_party_integration::rol::ROLVectorConstraintFunction{tConstraint};
}

std::unique_ptr<ROL::Problem<double>> create_rol_constrained_rosenbrock_problem_by_adding_two_constraints(
    ROL::Ptr<ROL::StdVector<double>>& aControls)
{
    auto tROLProblem = create_rol_bounded_rosenbrock_problem(aControls);
    auto tLine = create_line_constraint();
    add_linear_constraint_rol_problem(tROLProblem, tLine);
    auto tCircle = create_circle_constraint();
    add_nonlinear_constraint_rol_problem(tROLProblem, tCircle, 1U);

    finalize_rol_problem(tROLProblem);
    return tROLProblem;
}

std::unique_ptr<ROL::Problem<double>> create_rol_constrained_rosenbrock_problem_by_adding_one_vector_constraint(
    ROL::Ptr<ROL::StdVector<double>>& aControls)
{
    auto tROLProblem = create_rol_bounded_rosenbrock_problem(aControls);

    auto tConstraint = std::make_unique<third_party_integration::rol::ROLVectorConstraintFunction>(
        create_rol_vector_constraint_combination_line_and_circle());
    add_nonlinear_constraint_rol_problem(tROLProblem, tConstraint, 2U);

    finalize_rol_problem(tROLProblem);
    return tROLProblem;
}

auto solution_as_string(const linear_algebra::DynamicVector<double>& aSolution) -> std::string
{
    auto tStream = std::stringstream{};
    tStream << std::setprecision(16) << "tSolution[0]" << aSolution[0] << "\n";
    tStream << "tSolution[1]" << aSolution[1] << "\n";
    tStream << "tSolution[0]+tSolution[1] = " << aSolution[0] + aSolution[1] << "\n";
    tStream << "tSolution[0]^2+tSolution[1]^2  = " << aSolution[0] * aSolution[0] + aSolution[1] * aSolution[1] << "\n";
    tStream << "Rosenbrock: " << test_utilities::Rosenbrock{}.f(aSolution[0], aSolution[1]) << "\n";
    return tStream.str();
}

void print_and_test_solution(const linear_algebra::DynamicVector<double>& aSolution)
{
    const auto tSolutionForPrinting = solution_as_string(aSolution);
    ASSERT_EQ(aSolution.size(), 2u) << tSolutionForPrinting;
    EXPECT_NEAR(aSolution[0], kGoldXValue, kTolerance) << tSolutionForPrinting;
    EXPECT_NEAR(aSolution[1], kGoldYValue, kTolerance) << tSolutionForPrinting;
}

}  // namespace

TEST(Optimize, RosenbrockWithConstraintsROLTwoScalarConstraints)
{
    auto tControl = ROL::makePtr<ROL::StdVector<double>>(2, 0.);
    auto tROLInputs = create_parameter_list();
    auto tROLProblem = ROL::Ptr<ROL::Problem<double>>(
        create_rol_constrained_rosenbrock_problem_by_adding_two_constraints(tControl).release());

    auto tROLSolver = third_party_integration::rol::make_rol_solver(tROLInputs, tROLProblem);
    ROL::Ptr<std::ostream> tOutStream = ROL::makePtrFromRef(std::cout);
    tROLSolver.solve(*tOutStream);
    const auto tSolution = third_party_integration::rol::to_dynamic_vector(*tControl);
    print_and_test_solution(tSolution);
}

TEST(Optimize, RosenbrockWithConstraintsROLOneVectorConstraint)
{
    auto tControl = ROL::makePtr<ROL::StdVector<double>>(2, 0.);
    auto tROLInputs = create_parameter_list();
    auto tROLProblem = ROL::Ptr<ROL::Problem<double>>(
        create_rol_constrained_rosenbrock_problem_by_adding_one_vector_constraint(tControl).release());

    auto tROLSolver = third_party_integration::rol::make_rol_solver(tROLInputs, tROLProblem);
    ROL::Ptr<std::ostream> tOutStream = ROL::makePtrFromRef(std::cout);
    tROLSolver.solve(*tOutStream);
    const auto tSolution = third_party_integration::rol::to_dynamic_vector(*tControl);
    print_and_test_solution(tSolution);
}

}  // namespace plato::integration_tests::serial
