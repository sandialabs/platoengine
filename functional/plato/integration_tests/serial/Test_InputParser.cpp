#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "plato/input_parser/InputParser.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace
{
const std::filesystem::path kTestFileName = "testInput.i";
}

namespace plato::integration_tests::serial
{
TEST(InputParser, ParseFromFile)
{
    plato::test_utilities::create_input_file(kTestFileName);
    const input_parser::ParsedInput tInput = input_parser::parse_input_from_file(kTestFileName);

    EXPECT_FALSE(tInput.mOptimizationParameters.max_iterations.has_value());
    ASSERT_TRUE(tInput.mOptimizationParameters.gradient_tolerance.has_value());
    EXPECT_EQ(tInput.mOptimizationParameters.gradient_tolerance.value(), 100.0);
    ASSERT_TRUE(tInput.mOptimizationParameters.step_tolerance.has_value());
    EXPECT_EQ(tInput.mOptimizationParameters.step_tolerance.value(), 10.0);

    ASSERT_EQ(tInput.mObjectives.size(), 1);
    const input_parser::objective& tObjective = tInput.mObjectives.front();
    ASSERT_TRUE(tObjective.active.has_value());
    EXPECT_TRUE(tObjective.active.value());
    ASSERT_TRUE(tObjective.app.has_value());
    EXPECT_EQ(tObjective.app.value(), input_parser::CodeOptions::kNodalSum);
    ASSERT_TRUE(tObjective.number_of_processors.has_value());
    EXPECT_EQ(tObjective.number_of_processors.value(), 4);
    ASSERT_TRUE(tObjective.input_files.has_value());
    ASSERT_EQ(tObjective.input_files->mList.size(), 1);
    EXPECT_EQ(tObjective.input_files->mList.front(), "test-input.inp");
    ASSERT_TRUE(tObjective.aggregation_weight.has_value());
    EXPECT_EQ(tObjective.aggregation_weight.value(), 42.0);
    ASSERT_TRUE(tObjective.objective_type.has_value());
    EXPECT_EQ(tObjective.objective_type.value(), input_parser::ObjectiveTypes::kMinimize);

    EXPECT_EQ(tInput.mConstraints.size(), 0);

    ASSERT_TRUE(tInput.mBrickShapeGeometry.has_value());
    ASSERT_TRUE(tInput.mBrickShapeGeometry->mesh_name.has_value());
    EXPECT_EQ(tInput.mBrickShapeGeometry->mesh_name->mName, "my_mesh.exo");

    std::filesystem::remove(kTestFileName);
}
}  // namespace plato::integration_tests::serial
