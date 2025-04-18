#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/integration_tests/utilities/InputGeneration.hpp"
#include "plato/integration_tests/utilities/ValidInputTestFixture.hpp"
#include "plato/process_manager/extension/ROLOptimization.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::integration_tests::serial
{
namespace
{
struct InputParserTestFixture : public integration_tests::utilities::ValidInputTestFixture
{
};
}  // namespace

namespace
{
[[nodiscard]] auto input_deck() -> std::string
{
    return
        R"(
          begin brick_shape_geometry
            mesh_name my_mesh.exo
          end
          begin objective test
            active true
            app platoengine
            criterion nodal_sum
            number_of_processors 4
            input_files test-input.inp
            aggregation_weight 42.0
          end
          begin rol_optimization
            input_file_name its-a_file.txt
            step_tolerance 10
            gradient_tolerance 100.0
            
          end
       )";
}
}  // namespace

TEST(InputParser, ParseFromFile)
{
    const auto tInputOrError = input_parser::parse_to_new_input(input_deck());
    ASSERT_TRUE(tInputOrError.hasValue()) << tInputOrError.error();
    const auto& tInput = tInputOrError.value();

    const auto tROLOptimization = tInput.get<input_parser::rol_optimization>();
    ASSERT_EQ(tROLOptimization.size(), 1U);
    EXPECT_FALSE(tROLOptimization.front().max_iterations.has_value());
    EXPECT_EQ(tROLOptimization.front().gradient_tolerance.value(), 100.0);
    EXPECT_EQ(tROLOptimization.front().step_tolerance.value(), 10.0);

    const auto tObjectives = tInput.get<input_parser::objective>();
    ASSERT_EQ(tObjectives.size(), 1);
    const auto& tObjective = tObjectives.front();
    ASSERT_TRUE(tObjective.active.has_value());
    EXPECT_TRUE(tObjective.active.value());
    ASSERT_TRUE(tObjective.app.has_value());
    EXPECT_EQ(tObjective.app.value().mToken, "platoengine");
    ASSERT_TRUE(tObjective.criterion.has_value());
    EXPECT_EQ(tObjective.criterion.value().mToken, "nodal_sum");
    ASSERT_TRUE(tObjective.number_of_processors.has_value());
    EXPECT_EQ(tObjective.number_of_processors.value(), 4);
    ASSERT_TRUE(tObjective.input_files.has_value());
    ASSERT_EQ(tObjective.input_files->mList.size(), 1);
    EXPECT_EQ(tObjective.input_files->mList.front(), "test-input.inp");
    ASSERT_TRUE(tObjective.aggregation_weight.has_value());
    EXPECT_EQ(tObjective.aggregation_weight.value(), 42.0);

    EXPECT_TRUE(tInput.get<input_parser::constraint>().empty());

    const auto tBrickShapeGeometry = tInput.get<input_parser::brick_shape_geometry>();
    ASSERT_EQ(tBrickShapeGeometry.size(), 1U);
    ASSERT_TRUE(tBrickShapeGeometry.front().mesh_name.has_value());
    EXPECT_EQ(tBrickShapeGeometry.front().mesh_name->mToken, "my_mesh.exo");
}

TEST_F(InputParserTestFixture, IsValid)
{
    // Tests that the input provided by ValidInputTestFixture is actually valid
    const auto tValidatedInput = input_validation::make_validated_input(parsedInput());
    ASSERT_TRUE(tValidatedInput.hasValue());
    EXPECT_EQ(tValidatedInput.value().get<input_parser::ComponentType::kGeometry>().rawInput().mBlockName,
              "density_topology");
}
}  // namespace plato::integration_tests::serial
