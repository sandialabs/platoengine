#include <gtest/gtest.h>

#include <ROL_Algorithm.hpp>
#include <ROL_Solver.hpp>
#include <ROL_StdVector.hpp>

#include "plato/core/MeshProxy.hpp"
#include "plato/criteria/library/ObjectiveFactory.hpp"
#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/geometry/library/GeometryFactory.hpp"
#include "plato/main/library/PlatoProblem.hpp"
#include "plato/main/library/ValidatedInput.hpp"
#include "plato/optimizer/OptimizerFactory.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::main::library::unittest
{
TEST(PlatoProblem, ParsePlatoProblemEvaluateObjective)
{
    const std::string tInput = test_utilities::create_valid_brick_shape_geometry_string() +
                               test_utilities::create_valid_example_objective_string() +
                               test_utilities::create_valid_example_optimization_parameters_string();

    const ValidatedInput tData{parse_and_validate(tInput)};

    const PlatoProblem tProblem = make_plato_problem(tData);
    const auto tGeometry = geometry::library::make_geometry_data(tData.geometry());

    // Test Geometry
    const auto tBoundingBox = linear_algebra::DynamicVector{0.0, 0.0, 0.0, 1.0, 1.0, 1.0};
    const core::MeshProxy tGeomProxy = tGeometry.mCompute.f(tBoundingBox);
    const core::MeshProxy tPlatoProblemGeomProxy = tProblem.mGeometry.mCompute.f(tBoundingBox);
    EXPECT_EQ(tGeomProxy.mFileName, tPlatoProblemGeomProxy.mFileName);

    // Test Objective
    const auto tObjective = criteria::library::make_aggregate_objective_function(tData.objectives());
    EXPECT_EQ(tObjective.f(tGeomProxy), tProblem.mObjective.f(tGeomProxy));

    std::filesystem::remove(tGeomProxy.mFileName);
}

TEST(PlatoProblem, InputFileToROLObjective)
{
    constexpr double tWeight = 42.0;

    const std::string tInput = test_utilities::create_valid_brick_shape_geometry_string() +
                               " begin objective test"
                               " active true"
                               " app nodal_sum"
                               " number_of_processors 4"
                               " input_files test-input.inp"
                               " aggregation_weight " +
                               std::to_string(tWeight) + " objective_type minimize" + " end" +
                               test_utilities::create_valid_example_optimization_parameters_string();

    const ValidatedInput tData{parse_and_validate(tInput)};

    PlatoProblem tProblem = make_plato_problem(tData);
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

TEST(PlatoProblem, InputFileToROLConstraint)
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
                               test_utilities::create_valid_example_optimization_parameters_string();

    const ValidatedInput tData{parse_and_validate(tInput)};

    PlatoProblem tProblem = make_plato_problem(tData);
    const auto tConstraints = make_rol_constraints(tProblem);
    ASSERT_EQ(tConstraints.size(), 1);

    const ROL::StdVector<double> tBoundingBox{0, 0, 0, 1, 1, 1};
    ROL::StdVector<double> tResult{0};
    const auto tGold = std::vector{-2.0};
    double tTolerance = 1e-8;

    ASSERT_NE(tConstraints.front(), nullptr);
    tConstraints.front()->value(tResult, tBoundingBox, tTolerance);

    EXPECT_EQ(*tResult.getVector(), tGold);
    std::filesystem::remove("my_mesh.exo");
}

TEST(PlatoProblem, InputFileToROLSolver)
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
                               test_utilities::create_valid_example_optimization_parameters_string();

    const ValidatedInput tData{parse_and_validate(tInput)};
    const PlatoProblem tPlatoProblem = make_plato_problem(tData);
    Teuchos::ParameterList tROLOptions = tPlatoProblem.mROLOptions;
    const auto tROLProblem = Teuchos::RCP{make_rol_problem(tPlatoProblem).release()};
    const ROL::Solver<double> tSolver = optimizer::make_rol_solver(tROLOptions, std::move(tROLProblem));

    EXPECT_EQ(tSolver.getAlgorithmState()->iter, 0);
}

TEST(PlatoProblem, ParseAndValidateInvalidInput)
{
    const std::string tInput;
    EXPECT_THROW(const ValidatedInput tData = parse_and_validate(""), utilities::Exception);
}
}  // namespace plato::main::library::unittest
