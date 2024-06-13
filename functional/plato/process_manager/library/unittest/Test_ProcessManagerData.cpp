#include <gtest/gtest.h>

#include <filesystem>

#include "plato/core/MeshProxy.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::process_manager::library::unittest
{
TEST(ProcessManagerData, ParsePlatoProblemEvaluateObjective)
{
    const std::string tInput = test_utilities::create_valid_brick_shape_geometry_string() +
                               test_utilities::create_valid_example_objective_string() +
                               test_utilities::create_valid_example_rol_optimization_string();

    const ValidatedInput tData{parse_and_validate(tInput)};

    const ProcessManagerData tProblem = make_process_manager_data(tData);
    const auto tGeometry = geometry::library::make_geometry_data(tData.geometry());

    // Test Geometry
    const auto tBoundingBox = linear_algebra::DynamicVector{0.0, 0.0, 0.0, 1.0, 1.0, 1.0};
    const core::MeshProxy tGeomProxy = tGeometry.mCompute.f(tBoundingBox);
    const core::MeshProxy tPlatoProblemGeomProxy = tProblem.mGeometry.mCompute.f(tBoundingBox);
    EXPECT_TRUE(std::filesystem::exists(tGeomProxy.mFileName));

    // Test Objective
    const auto tObjective = criteria::library::make_aggregate_objective_function(tData.objectives());
    EXPECT_EQ(tObjective.f(tGeomProxy), tProblem.mObjective.f(tGeomProxy));
}

TEST(ProcessManagerData, ParseAndValidateInvalidInput)
{
    const std::string tInput;
    EXPECT_THROW(const ValidatedInput tData = parse_and_validate(""), utilities::Exception);
}
}  // namespace plato::process_manager::library::unittest
