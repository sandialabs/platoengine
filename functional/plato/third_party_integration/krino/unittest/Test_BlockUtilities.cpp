#include <gtest/gtest.h>

#include <Akri_MeshFromFile.hpp>
#include <algorithm>
#include <iterator>
#include <memory>
#include <stk_mesh/base/Types.hpp>
#include <stk_util/environment/EnvData.hpp>
#include <vector>

#include "plato/third_party_integration/krino/BlockUtilities.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::third_party_integration::krino::unittest
{
namespace
{
using krino::test_utilities::KrinoTestFixture;
}

TEST_F(KrinoTestFixture, BlockParts)
{
    constexpr auto tDecompositionMethod = std::string_view{"rib"};

    const auto tFourBlockMeshFilePath = utilities::data_file_path("four_block_rect_with_circ.cdf");
    ASSERT_TRUE(tFourBlockMeshFilePath.has_value());
    auto tMeshFromFile = std::make_unique<::krino::MeshFromFile>(
        tFourBlockMeshFilePath.value().string(), stk::EnvData::parallel_comm(), std::string{tDecompositionMethod});

    const auto tToBlockNames = [](const stk::mesh::PartVector& aBlockParts)
    {
        auto tBlockNames = utilities::reserved_container<std::vector<std::string>>(aBlockParts.size());
        std::ranges::transform(aBlockParts, std::back_inserter(tBlockNames),
                               [](const auto& aBlockPart) { return aBlockPart->name(); });
        return tBlockNames;
    };
    // All block parts
    const auto tAllBlockNames = std::vector<std::string>{"left_rect", "right_rect", "left_circ", "right_circ"};
    {
        const auto tAllBlocks = all_block_parts(*tMeshFromFile);
        EXPECT_EQ(tToBlockNames(tAllBlocks), tAllBlockNames);
    }
    // Subset of block parts
    {
        const auto tBlocks = all_blocks_except(*tMeshFromFile, {});
        EXPECT_EQ(tToBlockNames(tBlocks), tAllBlockNames);
    }
    {
        const auto tBlocks = all_blocks_except(*tMeshFromFile, {"right_rect"});
        const auto tExpectedBlockNames = std::vector<std::string>{"left_rect", "left_circ", "right_circ"};
        EXPECT_EQ(tToBlockNames(tBlocks), tExpectedBlockNames);
    }
    {
        const auto tBlocks = all_blocks_except(*tMeshFromFile, {"right_rect", "left_circ"});
        const auto tExpectedBlockNames = std::vector<std::string>{"left_rect", "right_circ"};
        EXPECT_EQ(tToBlockNames(tBlocks), tExpectedBlockNames);
    }
    {
        const auto tBlocks = all_blocks_except(*tMeshFromFile, {"left_circ", "right_circ", "right_rect"});
        const auto tExpectedBlockNames = std::vector<std::string>{"left_rect"};
        EXPECT_EQ(tToBlockNames(tBlocks), tExpectedBlockNames);
    }
    {
        const auto tBlocks = all_blocks_except(*tMeshFromFile, {"right_rect", "left_circ", "right_circ", "left_rect"});
        EXPECT_TRUE(tBlocks.empty());
    }
}
}  // namespace plato::third_party_integration::krino::unittest
