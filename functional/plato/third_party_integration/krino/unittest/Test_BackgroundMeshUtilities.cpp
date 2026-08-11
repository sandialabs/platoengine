#include <gtest/gtest.h>

#include <filesystem>
#include <set>
#include <stk_mesh/base/Types.hpp>
#include <string>
#include <vector>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/krino/BackgroundMeshUtilities.hpp"
#include "plato/third_party_integration/krino/CutMeshUtilities.hpp"
#include "plato/third_party_integration/krino/SnappingParameters.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::third_party_integration::krino::unittest
{
namespace
{
using krino::test_utilities::KrinoTestFixture;
constexpr double kOneQuarter{0.25};
constexpr double kThreeQuarters{0.75};
const auto kOneTriMeshFilePath = utilities::data_file_path("one_tri.cdf");
}  // namespace

TEST_F(KrinoTestFixture, BackgroundNodeIds)
{
    const auto tBackgroundMesh = test_utilities::background_mesh_from_vector(
        kOneTriMeshFilePath.value(), {kThreeQuarters, -kOneQuarter, -kOneQuarter});
    cut_mesh(tBackgroundMesh, SnappingParameters{});

    const auto tResult = background_node_ids(tBackgroundMesh);
    const auto tGold = std::vector<stk::mesh::EntityId>{1, 2, 4};
    EXPECT_EQ(tGold, tResult);
}

TEST_F(KrinoTestFixture, BackgroundNodeIdsWithExistingLevelSetField)
{
    // This test checks that a mesh with a field named `level_set` defined on it is read correctly.
    const auto tMeshFilePath = utilities::data_file_path("mesh_with_field.cdf");
    ASSERT_TRUE(tMeshFilePath);
    ASSERT_TRUE(std::filesystem::exists(tMeshFilePath.value()));

    const auto tFixedBlocks = std::set<std::string>{"fixed"};
    constexpr auto tDesignDomainSize = 44U;
    const auto tBackgroundMesh = test_utilities::background_mesh_from_vector(
        tMeshFilePath.value(), std::vector(tDesignDomainSize, 1.0), tFixedBlocks);

    const auto tResult = background_node_ids(tBackgroundMesh);
    EXPECT_EQ(tResult.size(), tDesignDomainSize);
}

TEST_F(KrinoTestFixture, BackgroundNodeIdsFourTri)
{
    test_utilities::four_tri_test_on_background_node_ids(TEST_CONTEXT("Four tri background nodes in serial."));
}
}  // namespace plato::third_party_integration::krino::unittest
