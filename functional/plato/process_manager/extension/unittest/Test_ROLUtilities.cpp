#include <gtest/gtest.h>

#include <ROL_Algorithm.hpp>
#include <ROL_Solver.hpp>
#include <ROL_StdVector.hpp>
#include <filesystem>
#include <memory>

#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/library/OutputManager.hpp"
#include "plato/process_manager/extension/ROLUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::process_manager::extension::unittest
{

TEST(ProcessManagerData, InputFileToROLObjective)
{
    constexpr double tWeight = 42.0;

    const std::string tInput = test_utilities::create_valid_brick_shape_geometry_string() +
                               " begin objective test"
                               " active true"
                               " criterion nodal_sum"
                               " number_of_processors 1"
                               " input_files test-input.inp"
                               " aggregation_weight " +
                               std::to_string(tWeight) + " end" +
                               test_utilities::create_valid_example_rol_optimization_string();

    const auto tData = input_validation::parse_and_validate_string(tInput);
    ASSERT_TRUE(tData.hasValue()) << tData.error();
    const auto tProblem = library::make_process_manager_data(tData.value());
    const auto tObjectiveFunction = make_rol_objective(tProblem);
    const auto tBoundingBox = std::vector<double>{0.5, 0.5, 0.5, 0.5, 0.5, 0.5};
    double tTolerance = 1e-8;

    constexpr double tNodalSum = 12.0;
    EXPECT_DOUBLE_EQ(tObjectiveFunction->value(tBoundingBox, tTolerance), tWeight * tNodalSum);

    geometry::extension::BrickDesign tDesign;
    EXPECT_EQ(tProblem.mGeometry.mInitialGuess.stdVector(),
              geometry::extension::detail::to_dynamic_vector(tDesign).stdVector());

    std::filesystem::remove("my_mesh.exo");
}

TEST(ProcessManagerData, InputFileToROLConstraint)
{
    const std::string tInput = test_utilities::create_valid_brick_shape_geometry_string() +
                               test_utilities::create_valid_example_objective_string() +
                               R"(
                                begin constraint test
                                  active true
                                  criterion nodal_sum
                                  constraint_value 2
                                  constraint_type equal_to
                                end
                              )" +
                               test_utilities::create_valid_example_rol_optimization_string();

    const auto tData = input_validation::parse_and_validate_string(tInput);
    ASSERT_TRUE(tData.hasValue()) << tData.error();
    auto tProblem = library::make_process_manager_data(tData.value());
    const auto tConstraints = make_rol_constraints(tProblem);
    ASSERT_EQ(tConstraints.size(), 1u);

    const std::vector<double> tBoundingBox{0, 0, 0, 1, 1, 1};
    std::vector<double> tResult{0};
    const auto tGold = std::vector{-2.0};
    double tTolerance = 1e-8;

    ASSERT_EQ(tConstraints.size(), 1u);
    ASSERT_NE(tConstraints.front().mConstraintFunction, nullptr);
    tConstraints.front().mConstraintFunction->value(tResult, tBoundingBox, tTolerance);

    EXPECT_EQ(tResult, tGold);
    std::filesystem::remove("my_mesh.exo");
}

namespace
{
void parse_and_generate_solver(const std::string& aInput, const plato::test_utilities::TestContext& aTestContext)
{
    const auto tData = input_validation::parse_and_validate_string(aInput);
    ASSERT_TRUE(tData.hasValue());
    const auto tPlatoProblem = library::make_process_manager_data(tData.value());
    const auto tValidatedOptimizationParameters =
        tData.value().get<input_parser::ComponentType::kProcessManager>().rawInput().front();
    auto tROLOptions = make_optimization_parameters(tValidatedOptimizationParameters).parameters();
    const auto tSolver = make_rol_solver(tROLOptions, make_rol_problem(tPlatoProblem).first);

    EXPECT_EQ(tSolver.getAlgorithmState()->iter, 0) << aTestContext;
}
}  // namespace

TEST(ProcessManagerData, InputFileToROLSolverEquality)
{
    const std::string tInput = test_utilities::create_valid_brick_shape_geometry_string() +
                               test_utilities::create_valid_example_objective_string() +
                               R"(
                                begin constraint test
                                  active true
                                  criterion nodal_sum
                                  constraint_value 2
                                  constraint_type equal_to
                                end
                              )" +
                               test_utilities::create_valid_example_rol_optimization_string();

    parse_and_generate_solver(tInput, TEST_CONTEXT("Equality constraint"));
}

TEST(ProcessManagerData, InputFileToROLSolverInequality)
{
    const std::string tInput = test_utilities::create_valid_brick_shape_geometry_string() +
                               test_utilities::create_valid_example_objective_string() +
                               R"(
                                begin constraint test
                                  active true
                                  criterion nodal_sum
                                  constraint_value 2
                                  constraint_type greater_than
                                end
                              )" +
                               test_utilities::create_valid_example_rol_optimization_string();

    parse_and_generate_solver(tInput, TEST_CONTEXT("Inequality constraint"));
}

}  // namespace plato::process_manager::extension::unittest
