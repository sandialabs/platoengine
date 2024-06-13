#include <gtest/gtest.h>

#include <ROL_Algorithm.hpp>
#include <ROL_Solver.hpp>
#include <ROL_StdVector.hpp>
#include <filesystem>
#include <memory>

#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/process_manager/extension/ROLUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::process_manager::extension::unittest
{
TEST(ProcessManagerData, InputFileToROLObjective)
{
    constexpr double tWeight = 42.0;

    const std::string tInput = test_utilities::create_valid_brick_shape_geometry_string() +
                               " begin objective test"
                               " active true"
                               " app nodal_sum"
                               " number_of_processors 1"
                               " input_files test-input.inp"
                               " aggregation_weight " +
                               std::to_string(tWeight) + " objective_type minimize" + " end" +
                               test_utilities::create_valid_example_rol_optimization_string();

    const library::ValidatedInput tData{library::parse_and_validate(tInput)};

    const library::ProcessManagerData tProblem = library::make_process_manager_data(tData);
    std::unique_ptr<rol_integration::ROLObjectiveFunction> tObjectiveFunction = make_rol_objective(tProblem);
    const ROL::StdVector<double> tBoundingBox{0.5, 0.5, 0.5, 0.5, 0.5, 0.5};
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
                                  app nodal_sum
                                  equal_to 2
                                end
                              )" +
                               test_utilities::create_valid_example_rol_optimization_string();

    const library::ValidatedInput tData{library::parse_and_validate(tInput)};

    library::ProcessManagerData tProblem = library::make_process_manager_data(tData);
    const auto tConstraints = make_rol_constraints(tProblem);
    ASSERT_EQ(tConstraints.size(), 1u);

    const ROL::StdVector<double> tBoundingBox{0, 0, 0, 1, 1, 1};
    ROL::StdVector<double> tResult{0};
    const auto tGold = std::vector{-2.0};
    double tTolerance = 1e-8;

    ASSERT_NE(tConstraints.front(), nullptr);
    tConstraints.front()->value(tResult, tBoundingBox, tTolerance);

    EXPECT_EQ(*tResult.getVector(), tGold);
    std::filesystem::remove("my_mesh.exo");
}

TEST(ProcessManagerData, InputFileToROLSolver)
{
    const std::string tInput = test_utilities::create_valid_brick_shape_geometry_string() +
                               test_utilities::create_valid_example_objective_string() +
                               R"(
                                begin constraint test
                                  active true
                                  app nodal_sum
                                  equal_to 2
                                end
                              )" +
                               test_utilities::create_valid_example_rol_optimization_string();

    const library::ValidatedInput tData{library::parse_and_validate(tInput)};
    const library::ProcessManagerData tPlatoProblem = library::make_process_manager_data(tData);
    const auto tValidatedOptimizationParameters = library::process_manager_input<input_parser::rol_optimization>(
        tData.processManagers().rawInput().front());
    Teuchos::ParameterList tROLOptions = rol_integration::rol_parameter_list(tValidatedOptimizationParameters);
    const auto tROLProblem = Teuchos::RCP{make_rol_problem(tPlatoProblem).release()};
    const ROL::Solver<double> tSolver = rol_integration::make_rol_solver(tROLOptions, std::move(tROLProblem));

    EXPECT_EQ(tSolver.getAlgorithmState()->iter, 0);
}

}  // namespace plato::process_manager::extension::unittest