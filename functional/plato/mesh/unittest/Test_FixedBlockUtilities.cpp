#include <gtest/gtest.h>

#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/mesh/FixedBlockUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

// clang-format off
PLATO_GEOMETRY_INPUT_BLOCK_STRUCT(
    (plato)(input_parser),
    test_geometry,
    (plato::input_parser::FileName, mesh_name, "")
    (plato::input_parser::BlockList, fixed_blocks, "")
    )
// clang-format on

namespace plato::mesh::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::TwoDTwoBlockMesh;

const auto kTestGeometry =
    input_parser::test_geometry{/*.mesh_name=*/input_parser::FileName{"test.exo"}, /*.fixed_blocks=*/boost::none};

constexpr auto kTestGeometryMeshNameAccessor = [](const auto& aInput) { return aInput.mesh_name; };
}  // namespace

TEST(FixedBlockUtilities, UniqueFixedBlockNames)
{
    const auto tBlockName1 = std::string{"block_1"};
    const auto tBlockName2 = std::string{"some-other-block"};
    {
        auto tGeometryInputWithFixedBlocks = kTestGeometry;
        tGeometryInputWithFixedBlocks.fixed_blocks = input_parser::BlockList{{tBlockName1}};
        const auto tUniqueFixedBlocks = fixed_blocks(tGeometryInputWithFixedBlocks);
        EXPECT_EQ(tUniqueFixedBlocks.count(tBlockName1), 1U);
    }
    {
        auto tGeometryInputWithFixedBlocks = kTestGeometry;
        tGeometryInputWithFixedBlocks.fixed_blocks =
            input_parser::BlockList{{tBlockName1, tBlockName2, tBlockName1, tBlockName2}};
        const auto tUniqueFixedBlocks = fixed_blocks(tGeometryInputWithFixedBlocks);
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
        auto tGeometryInputWithFixedBlocks = kTestGeometry;
        tGeometryInputWithFixedBlocks.fixed_blocks = input_parser::BlockList{{}};
        EXPECT_FALSE(validate_unique_fixed_block_names(tGeometryInputWithFixedBlocks, kTestGeometryMeshNameAccessor)
                         .has_value());

        tGeometryInputWithFixedBlocks.fixed_blocks = input_parser::BlockList{{tBlockName1}};
        EXPECT_FALSE(validate_unique_fixed_block_names(tGeometryInputWithFixedBlocks, kTestGeometryMeshNameAccessor)
                         .has_value());

        tGeometryInputWithFixedBlocks.fixed_blocks = input_parser::BlockList{{tBlockName1, tBlockName2}};
        EXPECT_FALSE(validate_unique_fixed_block_names(tGeometryInputWithFixedBlocks, kTestGeometryMeshNameAccessor)
                         .has_value());
    }
    // Invalid
    {
        auto tGeometryInputWithFixedBlocks = kTestGeometry;
        tGeometryInputWithFixedBlocks.fixed_blocks = input_parser::BlockList{{tBlockName1, tBlockName1}};
        EXPECT_TRUE(validate_unique_fixed_block_names(tGeometryInputWithFixedBlocks, kTestGeometryMeshNameAccessor)
                        .has_value());

        tGeometryInputWithFixedBlocks.fixed_blocks = input_parser::BlockList{{tBlockName1, tBlockName2, tBlockName1}};
        EXPECT_TRUE(validate_unique_fixed_block_names(tGeometryInputWithFixedBlocks, kTestGeometryMeshNameAccessor)
                        .has_value());

        tGeometryInputWithFixedBlocks.fixed_blocks = input_parser::BlockList{{tBlockName2, tBlockName2, tBlockName1}};
        EXPECT_TRUE(validate_unique_fixed_block_names(tGeometryInputWithFixedBlocks, kTestGeometryMeshNameAccessor)
                        .has_value());
    }
}

TEST_F(TwoDTwoBlockMesh, ValidateBlockNamesExist)
{
    const auto tFixedBlockName = std::string{mBlockNames[0]};
    const auto tDesignBlockName = std::string{mBlockNames[1]};
    const auto tBogusBlockName = std::string{"bogus"};

    auto tGeometryInputWithFixedBlocks = kTestGeometry;
    tGeometryInputWithFixedBlocks.mesh_name = input_parser::FileName{mMeshFilePath.string()};

    // Valid
    {
        tGeometryInputWithFixedBlocks.fixed_blocks = input_parser::BlockList{{}};
        const auto tErrorMessageForNoFixedBlocks =
            validate_fixed_block_names_exist(tGeometryInputWithFixedBlocks, kTestGeometryMeshNameAccessor);
        EXPECT_FALSE(tErrorMessageForNoFixedBlocks.has_value()) << tErrorMessageForNoFixedBlocks.value();

        tGeometryInputWithFixedBlocks.fixed_blocks = input_parser::BlockList{{tFixedBlockName}};
        const auto tErrorMessageForOneFixedBlock =
            validate_fixed_block_names_exist(tGeometryInputWithFixedBlocks, kTestGeometryMeshNameAccessor);
        EXPECT_FALSE(tErrorMessageForOneFixedBlock.has_value()) << tErrorMessageForOneFixedBlock.value();

        tGeometryInputWithFixedBlocks.fixed_blocks = input_parser::BlockList{{tFixedBlockName, tDesignBlockName}};
        const auto tErrorMessageForTwoFixedBlocks =
            validate_fixed_block_names_exist(tGeometryInputWithFixedBlocks, kTestGeometryMeshNameAccessor);
        EXPECT_FALSE(tErrorMessageForTwoFixedBlocks.has_value()) << tErrorMessageForTwoFixedBlocks.value();
    }
    // Invalid
    {
        tGeometryInputWithFixedBlocks.fixed_blocks = input_parser::BlockList{{tBogusBlockName}};
        EXPECT_TRUE(
            validate_fixed_block_names_exist(tGeometryInputWithFixedBlocks, kTestGeometryMeshNameAccessor).has_value());
    }
}

TEST_F(TwoDTwoBlockMesh, ValidateAtLeastOneDesignBlock)
{
    const auto tFixedBlockName = std::string{mBlockNames[0]};
    const auto tDesignBlockName = std::string{mBlockNames[1]};
    auto tGeometryInputWithFixedBlocks = kTestGeometry;
    tGeometryInputWithFixedBlocks.mesh_name = input_parser::FileName{mMeshFilePath.string()};

    // Valid
    {
        tGeometryInputWithFixedBlocks.fixed_blocks = input_parser::BlockList{{}};
        const auto tErrorMessageForNoFixedBlocks =
            validate_at_least_one_design_block(tGeometryInputWithFixedBlocks, kTestGeometryMeshNameAccessor);
        EXPECT_FALSE(tErrorMessageForNoFixedBlocks.has_value()) << tErrorMessageForNoFixedBlocks.value();

        tGeometryInputWithFixedBlocks.fixed_blocks = input_parser::BlockList{{tFixedBlockName}};
        const auto tErrorMessageForOneFixedBlock =
            validate_at_least_one_design_block(tGeometryInputWithFixedBlocks, kTestGeometryMeshNameAccessor);
        EXPECT_FALSE(tErrorMessageForOneFixedBlock.has_value()) << tErrorMessageForOneFixedBlock.value();
    }
    // Invalid
    {
        tGeometryInputWithFixedBlocks.fixed_blocks = input_parser::BlockList{{tFixedBlockName, tDesignBlockName}};
        const auto tErrorMessageForAllFixedBlock =
            validate_at_least_one_design_block(tGeometryInputWithFixedBlocks, kTestGeometryMeshNameAccessor);
        EXPECT_TRUE(tErrorMessageForAllFixedBlock.has_value()) << tErrorMessageForAllFixedBlock.value();
    }
}

}  // namespace plato::mesh::unittest
