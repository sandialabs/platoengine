#include <gtest/gtest.h>

#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/extension/FixedBlockUtilities.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::geometry::extension::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::TwoDTwoBlockMesh;

const auto kDensityTopology = geometry::extension::create_valid_density_topology_geometry_input();

constexpr auto kDensityTopologyMeshNameAccessor = [](const auto& aInput) { return aInput.mesh_name; };
}  // namespace

TEST(FixedBlockUtilities, UniqueFixedBlockNames)
{
    const auto tBlockName1 = std::string{"block_1"};
    const auto tBlockName2 = std::string{"some-other-block"};
    {
        auto tDensityInputWithFixedBlocks = kDensityTopology;
        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tBlockName1}};
        const auto tUniqueFixedBlocks = fixed_blocks(tDensityInputWithFixedBlocks);
        EXPECT_EQ(tUniqueFixedBlocks.count(tBlockName1), 1U);
    }
    {
        auto tDensityInputWithFixedBlocks = kDensityTopology;
        tDensityInputWithFixedBlocks.fixed_blocks =
            input_parser::FixedBlockList{{tBlockName1, tBlockName2, tBlockName1, tBlockName2}};
        const auto tUniqueFixedBlocks = fixed_blocks(tDensityInputWithFixedBlocks);
        EXPECT_EQ(tUniqueFixedBlocks.count(tBlockName1), 1U);
        EXPECT_EQ(tUniqueFixedBlocks.count(tBlockName2), 1U);
    }
}

TEST(FixedBlockUtilities, ValidateUniqueBlockNames)
{
    const auto tBlockName1 = std::string{"fixed-block-1"};
    const auto tBlockName2 = std::string{"fixed-block-2"};
    const auto tBlockName3 = std::string{"design-block"};
    // Valid cases
    {
        auto tDensityInputWithFixedBlocks = kDensityTopology;
        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{}};
        EXPECT_FALSE(validate_unique_fixed_block_names(tDensityInputWithFixedBlocks, kDensityTopologyMeshNameAccessor)
                         .has_value());

        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tBlockName1}};
        EXPECT_FALSE(validate_unique_fixed_block_names(tDensityInputWithFixedBlocks, kDensityTopologyMeshNameAccessor)
                         .has_value());

        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tBlockName1, tBlockName2}};
        EXPECT_FALSE(validate_unique_fixed_block_names(tDensityInputWithFixedBlocks, kDensityTopologyMeshNameAccessor)
                         .has_value());
    }
    // Invalid
    {
        auto tDensityInputWithFixedBlocks = kDensityTopology;
        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tBlockName1, tBlockName1}};
        EXPECT_TRUE(validate_unique_fixed_block_names(tDensityInputWithFixedBlocks, kDensityTopologyMeshNameAccessor)
                        .has_value());

        tDensityInputWithFixedBlocks.fixed_blocks =
            input_parser::FixedBlockList{{tBlockName1, tBlockName2, tBlockName1}};
        EXPECT_TRUE(validate_unique_fixed_block_names(tDensityInputWithFixedBlocks, kDensityTopologyMeshNameAccessor)
                        .has_value());

        tDensityInputWithFixedBlocks.fixed_blocks =
            input_parser::FixedBlockList{{tBlockName2, tBlockName2, tBlockName1}};
        EXPECT_TRUE(validate_unique_fixed_block_names(tDensityInputWithFixedBlocks, kDensityTopologyMeshNameAccessor)
                        .has_value());
    }
}

TEST_F(TwoDTwoBlockMesh, ValidateBlockNamesExist)
{
    const auto tFixedBlockName = std::string{"fixed"};
    const auto tDesignBlockName = std::string{"design"};
    const auto tBogusBlockName = std::string{"bogus"};

    auto tDensityInputWithFixedBlocks = kDensityTopology;
    tDensityInputWithFixedBlocks.mesh_name = input_parser::FileName{mMeshFilePath.string()};

    // Valid
    {
        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{}};
        const auto tErrorMessageForNoFixedBlocks =
            validate_fixed_block_names_exist(tDensityInputWithFixedBlocks, kDensityTopologyMeshNameAccessor);
        EXPECT_FALSE(tErrorMessageForNoFixedBlocks.has_value()) << tErrorMessageForNoFixedBlocks.value();

        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tFixedBlockName}};
        const auto tErrorMessageForOneFixedBlock =
            validate_fixed_block_names_exist(tDensityInputWithFixedBlocks, kDensityTopologyMeshNameAccessor);
        EXPECT_FALSE(tErrorMessageForOneFixedBlock.has_value()) << tErrorMessageForOneFixedBlock.value();

        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tFixedBlockName, tDesignBlockName}};
        const auto tErrorMessageForTwoFixedBlocks =
            validate_fixed_block_names_exist(tDensityInputWithFixedBlocks, kDensityTopologyMeshNameAccessor);
        EXPECT_FALSE(tErrorMessageForTwoFixedBlocks.has_value()) << tErrorMessageForTwoFixedBlocks.value();
    }
    // Invalid
    {
        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tBogusBlockName}};
        EXPECT_TRUE(validate_fixed_block_names_exist(tDensityInputWithFixedBlocks, kDensityTopologyMeshNameAccessor)
                        .has_value());
    }
}

TEST_F(TwoDTwoBlockMesh, ValidateAtLeastOneDesignBlock)
{
    const auto tFixedBlockName = std::string{"fixed"};
    const auto tDesignBlockName = std::string{"design"};
    auto tDensityInputWithFixedBlocks = kDensityTopology;
    tDensityInputWithFixedBlocks.mesh_name = input_parser::FileName{mMeshFilePath.string()};

    // Valid
    {
        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{}};
        const auto tErrorMessageForNoFixedBlocks =
            validate_at_least_one_design_block(tDensityInputWithFixedBlocks, kDensityTopologyMeshNameAccessor);
        EXPECT_FALSE(tErrorMessageForNoFixedBlocks.has_value()) << tErrorMessageForNoFixedBlocks.value();

        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tFixedBlockName}};
        const auto tErrorMessageForOneFixedBlock =
            validate_at_least_one_design_block(tDensityInputWithFixedBlocks, kDensityTopologyMeshNameAccessor);
        EXPECT_FALSE(tErrorMessageForOneFixedBlock.has_value()) << tErrorMessageForOneFixedBlock.value();
    }
    // Invalid
    {
        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tFixedBlockName, tDesignBlockName}};
        const auto tErrorMessageForAllFixedBlock =
            validate_at_least_one_design_block(tDensityInputWithFixedBlocks, kDensityTopologyMeshNameAccessor);
        EXPECT_TRUE(tErrorMessageForAllFixedBlock.has_value()) << tErrorMessageForAllFixedBlock.value();
    }
}

}  // namespace plato::geometry::extension::unittest
