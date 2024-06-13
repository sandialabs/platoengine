#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "plato/core/InputVariantUtilities.hpp"
#include "plato/input_parser/InputParser.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::integration_tests::serial
{

namespace
{
const std::filesystem::path kTestFileName = "testInput.i";
void create_input_file(const std::filesystem::path& aTestFileName)
{
    std::ofstream tOutFile(aTestFileName);
    const std::string tInput =
        R"(
          begin brick_shape_geometry
            mesh_name my_mesh.exo
          end
          begin objective test
            active true
            app nodal_sum
            number_of_processors 4
            input_files test-input.inp
            aggregation_weight 42.0
            objective_type minimize
          end
          begin rol_optimization
            input_file_name its-a_file.txt
            step_tolerance 10
            gradient_tolerance 100.0
            
          end
       )";
    tOutFile << tInput << std::endl;
    tOutFile.close();
}
}  // namespace

TEST(InputParser, ParseFromFile)
{
    create_input_file(kTestFileName);
    const input_parser::ParsedInput tInput = input_parser::parse_input_from_file(kTestFileName);
    ASSERT_TRUE(tInput.mROLOptimization);
    EXPECT_FALSE(tInput.mROLOptimization->max_iterations.has_value());
    ASSERT_TRUE(tInput.mROLOptimization->gradient_tolerance.has_value());
    EXPECT_EQ(tInput.mROLOptimization->gradient_tolerance.value(), 100.0);
    ASSERT_TRUE(tInput.mROLOptimization->step_tolerance.has_value());
    EXPECT_EQ(tInput.mROLOptimization->step_tolerance.value(), 10.0);

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

TEST(ParsedInputVariant, BlockName)
{
    const process_manager::library::ValidatedInput tValidatedInput =
        process_manager::library::make_validated_input(plato::test_utilities::create_valid_example_input());
    EXPECT_EQ(core::block_name(tValidatedInput.geometry().rawInput()), "density_topology");
}
}  // namespace plato::integration_tests::serial
