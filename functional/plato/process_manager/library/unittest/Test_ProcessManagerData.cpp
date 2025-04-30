#include <gtest/gtest.h>

#include <filesystem>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::process_manager::library::unittest
{
TEST(ProcessManagerData, ParsePlatoProblemEvaluateObjective)
{
    const std::string tInput = test_utilities::create_valid_brick_shape_geometry_string() +
                               test_utilities::create_valid_example_objective_string() +
                               test_utilities::create_valid_example_rol_optimization_string();

    const auto tData = input_validation::parse_and_validate_string(tInput);
    ASSERT_TRUE(tData.hasValue());
    const auto tProblem = make_process_manager_data(tData.value());
    const auto tGeometry =
        geometry::library::make_geometry_data(tData.value().get<input_parser::ComponentType::kGeometry>());

    // Test Geometry
    const auto tBoundingBox = linear_algebra::DynamicVector{0.0, 0.0, 0.0, 1.0, 1.0, 1.0};
    const analysis::AnalysisDomainMesh tGeomProxy =
        tGeometry.mCompute.evaluate<core::evaluation::kFunction>(tBoundingBox);
    const analysis::AnalysisDomainMesh tPlatoProblemGeomProxy =
        tProblem.mGeometry.mCompute.evaluate<core::evaluation::kFunction>(tBoundingBox);
    EXPECT_TRUE(std::filesystem::exists(tGeomProxy.mFileName));

    // Test Objective
    const auto tObjective = criteria::library::make_aggregate_objective_function(
        tData.value().get<input_parser::ComponentType::kObjective>());
    EXPECT_EQ(tObjective.evaluate<core::evaluation::kFunction>(tGeomProxy),
              tProblem.mObjective.evaluate<core::evaluation::kFunction>(tGeomProxy));
}

TEST(ProcessManagerData, ParseAndValidateInvalidInput)
{
    const auto tData = input_validation::parse_and_validate_string("");
    EXPECT_TRUE(tData.hasError());
}
}  // namespace plato::process_manager::library::unittest
