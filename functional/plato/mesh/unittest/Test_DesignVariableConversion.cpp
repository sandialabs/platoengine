#include <gtest/gtest.h>

#include <numeric>
#include <set>

#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshBlocks.hpp"
#include "plato/mesh/MeshDesignVariablesViews.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/utilities/Enumerate.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::mesh::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;
using third_party_integration::stk_io::test_utilities::TwoBlockMeshOnDisk;
using third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;

void check_sizes(const MeshDesignVariables& aMeshDesignVariables,
                 const std::vector<unsigned int>& aExpectedEntitySizes,
                 const test_utilities::TestContext& aTestContext)
{
    EXPECT_EQ(aMeshDesignVariables.mBlockDensities.size(), aExpectedEntitySizes.size()) << aTestContext;
    for (const auto& [tDesignVariables, tSize] :
         utilities::Zip{aMeshDesignVariables.mBlockDensities, aExpectedEntitySizes})
    {
        EXPECT_EQ(tDesignVariables.second.size(), tSize) << aTestContext;
    }
}

void check_id_matches_density(const MeshDesignVariables& aMeshDesignVariables,
                              const test_utilities::TestContext& aTestContext)
{
    for (const auto& [tBlockID, tDensities] : aMeshDesignVariables.mBlockDensities)
    {
        for (const auto& tResult : tDensities)
        {
            EXPECT_EQ(tResult.mDensity, tResult.mGlobalMeshEntityID) << aTestContext;
        }
    }
}

void check_vector_index_matches_view_index(const MeshDesignVariables& aMeshDesignVariables,
                                           const test_utilities::TestContext& aTestContext)
{
    const auto tDesignVariablesView = mesh::MeshDesignVariablesDensitiesView{aMeshDesignVariables};
    for (const auto& [tIndex, tDensityFromView] : utilities::enumerate(tDesignVariablesView))
    {
        const auto& tDensity = static_cast<mesh::Density>(tDensityFromView);
        EXPECT_EQ(tIndex, tDensity.mDesignVariableVectorIndex) << aTestContext;
    }
}

std::vector<double> entity_ids_to_densities(const std::vector<std::size_t>& aEntityIDs)
{
    auto tUniqueIDs = std::set<std::size_t>{};
    std::copy(aEntityIDs.cbegin(), aEntityIDs.cend(), std::inserter(tUniqueIDs, tUniqueIDs.begin()));
    auto tDensities = std::vector<double>();
    tDensities.reserve(tUniqueIDs.size());
    std::transform(tUniqueIDs.cbegin(), tUniqueIDs.cend(), std::back_inserter(tDensities),
                   [](const std::size_t aIndex) { return static_cast<double>(aIndex); });
    return tDensities;
}

void check_map_consistency(const DesignVariablesConversion& tMesh,
                           const std::size_t aNumberOfDesignNodes,
                           const test_utilities::TestContext& aTestContext)
{
    auto tNodalIDs = std::vector<std::size_t>(aNumberOfDesignNodes);
    std::iota(tNodalIDs.begin(), tNodalIDs.end(), 1);
    const auto tDensities = entity_ids_to_densities(tNodalIDs);
    const auto tDensityMap = tMesh.nodalDensitiesToNodalIDMap(NodalDensityVectorReference{tDensities});
    for (const auto tNodalID : tNodalIDs)
    {
        const auto tNodalDensityMapIterator = tDensityMap.find(tNodalID);
        ASSERT_NE(tNodalDensityMapIterator, tDensityMap.end()) << aTestContext;
        EXPECT_EQ(tNodalDensityMapIterator->first, tNodalDensityMapIterator->second) << aTestContext;
    }
}

MeshDesignVariables one_block_mesh_design_variables_for_tests(
    const third_party_integration::stk_io::CommandGenerator& aCommandGenerator,
    const std::filesystem::path& aMeshFilePath)
{
    auto tDesignVariables = std::vector<Density>{};
    tDesignVariables.reserve(aCommandGenerator.numberOfNodes());
    std::generate_n(std::back_inserter(tDesignVariables), aCommandGenerator.numberOfNodes(),
                    [tCount = Density::IndexType{0}]() mutable
                    {
                        ++tCount;
                        return Density{tCount, tCount - 1, static_cast<double>(tCount)};
                    });
    return MeshDesignVariables{aMeshFilePath, {{0, tDesignVariables}}};
}

}  // namespace

TEST_F(TwoBlockMeshOnDisk, NodalDensitiesToMeshDesignVariables)
{
    const auto tMesh = Mesh{mMeshFilePath};
    auto tDensities = std::vector<double>(EntityCounts{tMesh}.numberOfNodes());
    constexpr auto tStartDensity = double{1.0};
    std::iota(tDensities.begin(), tDensities.end(), tStartDensity);
    const auto tMeshDesignVariables = DesignVariablesConversion{tMesh}.nodalDensitiesToMeshDesignVariables(
        NodalDensityVectorReference{std::cref(tDensities)});

    check_sizes(tMeshDesignVariables, {mExpectedNumberOfNodesInBlock1, mExpectedNumberOfNodesInBlock2},
                TEST_CONTEXT("Nodal design variables"));
    check_id_matches_density(tMeshDesignVariables, TEST_CONTEXT("Nodal densities"));
    check_vector_index_matches_view_index(tMeshDesignVariables, TEST_CONTEXT("Nodal design variables"));
}

TEST_F(TwoBlockMeshOnDisk, ElementDensitiesToMeshDesignVariables)
{
    const auto tMesh = Mesh{mMeshFilePath};
    auto tDensities = std::vector<double>(EntityCounts{tMesh}.numberOfElements());
    constexpr auto tStartDensity = double{1.0};
    std::iota(tDensities.begin(), tDensities.end(), tStartDensity);

    const auto tMeshDesignVariables = DesignVariablesConversion{tMesh}.elementDensitiesToMeshDesignVariables(
        ElementDensityVectorReference{tDensities});

    check_sizes(tMeshDesignVariables, {mExpectedNumberOfElementsInBlock1, mExpectedNumberOfElementsInBlock2},
                TEST_CONTEXT("Element design variables"));
    check_id_matches_density(tMeshDesignVariables, TEST_CONTEXT("Element design variables"));
    check_vector_index_matches_view_index(tMeshDesignVariables, TEST_CONTEXT("Element design variables"));
}

TEST_F(TwoDThreeBlockMesh, NodeDensitiesToDesignVariablesWithFixedBlocks)
{
    const auto tMesh = Mesh{mMeshFilePath, {"block_3"}};
    auto tNodeIds = MeshBlocks{tMesh}.nodeIDs(mBlock1Ordinal);
    const auto tNodeIdsBlock2 = MeshBlocks{tMesh}.nodeIDs(mBlock2Ordinal);
    tNodeIds.insert(tNodeIds.end(), tNodeIdsBlock2.cbegin(), tNodeIdsBlock2.cend());
    const auto tDensities = entity_ids_to_densities(tNodeIds);

    const auto tMeshDesignVariables =
        DesignVariablesConversion{tMesh}.nodalDensitiesToMeshDesignVariables(NodalDensityVectorReference{tDensities});

    check_sizes(tMeshDesignVariables, {mExpectedNumberOfNodesInBlock1, mExpectedNumberOfNodesInBlock2},
                TEST_CONTEXT("Nodal design variables"));
    check_id_matches_density(tMeshDesignVariables, TEST_CONTEXT("Nodal design variables"));
    check_vector_index_matches_view_index(tMeshDesignVariables, TEST_CONTEXT("Nodal design variables"));
}

TEST_F(TwoDThreeBlockMesh, ElementDensitiesToDesignVariablesWithFixedBlocks)
{
    const auto tMesh = Mesh{mMeshFilePath, {"block_1", "block_2"}};
    const auto tElementIds = MeshBlocks{tMesh}.elementIDs(mBlock3Ordinal);
    const auto tDensities = entity_ids_to_densities(tElementIds);
    const auto tMeshDesignVariables = DesignVariablesConversion{tMesh}.elementDensitiesToMeshDesignVariables(
        ElementDensityVectorReference{tDensities});

    check_sizes(tMeshDesignVariables, {mExpectedNumberOfElementsInBlock3}, TEST_CONTEXT("Element design variables"));
    check_id_matches_density(tMeshDesignVariables, TEST_CONTEXT("Element design variables"));
    check_vector_index_matches_view_index(tMeshDesignVariables, TEST_CONTEXT("Element design variables"));
}

TEST_F(OneBlock3x1x1HexMesh, MeshDesignVariablesToNodalDensities)
{
    const auto tMeshDesignVariables = one_block_mesh_design_variables_for_tests(mCommandGenerator, mMeshFilePath);

    const auto tMesh = Mesh{mMeshFilePath, {}};
    const auto tDesignVariableVector =
        DesignVariablesConversion{tMesh}.meshDesignVariablesToNodalDensityVector(tMeshDesignVariables);

    auto tExpectedIndices = std::vector<Density::IndexType>(mCommandGenerator.numberOfNodes());
    constexpr auto tStartIndex = Density::IndexType{1};
    std::iota(tExpectedIndices.begin(), tExpectedIndices.end(), tStartIndex);

    EXPECT_EQ(tDesignVariableVector.mValue.size(), tExpectedIndices.size());
    for (const auto& [tResult, tExpected] : utilities::Zip{tDesignVariableVector.mValue, tExpectedIndices})
    {
        EXPECT_EQ(static_cast<double>(tExpected), tResult);
    }
}

TEST_F(TwoDThreeBlockMesh, MeshDesignVariablesToElementDensities)
{
    // Set up to assume blocks 1 and 2 are fixed
    constexpr auto tBlock3ElementGlobalID = Density::IndexType{3};
    constexpr auto tVectorIndex = Density::IndexType{0};
    constexpr auto tArbitraryDensity = double{0.5};
    const auto tDesignVariables = std::vector<Density>{{tBlock3ElementGlobalID, tVectorIndex, tArbitraryDensity}};
    constexpr auto tBlock3ID = MeshDesignVariables::BlockIDType{3};
    const auto tMeshDesignVariables = MeshDesignVariables{mMeshFilePath, {{tBlock3ID, tDesignVariables}}};

    const auto tMesh = Mesh{mMeshFilePath, {"block_1", "block_2"}};
    const auto tResultDesignVariableVector =
        DesignVariablesConversion{tMesh}.meshDesignVariablesToElementDensityVector(tMeshDesignVariables);

    const auto tExpectedDesignVariableVector = std::vector{tArbitraryDensity};
    EXPECT_EQ(tResultDesignVariableVector.mValue, tExpectedDesignVariableVector);
}

TEST_F(TwoDThreeBlockMesh, MeshNodalDesignVariablesRoundTrip)
{
    const auto tMesh = Mesh{mMeshFilePath, {"block_3"}};
    auto tNodalDesignVariables = std::vector<double>(EntityCounts{tMesh}.numberOfDesignDomainNodes());
    constexpr auto tStartDensity = double{1.0};
    std::iota(tNodalDesignVariables.begin(), tNodalDesignVariables.end(), tStartDensity);

    const auto tMeshDesignVariables = DesignVariablesConversion{tMesh}.nodalDensitiesToMeshDesignVariables(
        NodalDensityVectorReference{tNodalDesignVariables});
    const auto tRoundTripNodalDesignVariables =
        DesignVariablesConversion{tMesh}.meshDesignVariablesToNodalDensityVector(tMeshDesignVariables);

    EXPECT_EQ(tNodalDesignVariables, tRoundTripNodalDesignVariables.mValue);
}

TEST_F(TwoDThreeBlockMesh, MeshElementDesignVariablesRoundTrip)
{
    const auto tMesh = Mesh{mMeshFilePath, {"block_1"}};
    auto tElementDesignVariables = std::vector<double>(EntityCounts{tMesh}.numberOfDesignDomainElements());
    constexpr auto tStartDensity = double{1.0};
    std::iota(tElementDesignVariables.begin(), tElementDesignVariables.end(), tStartDensity);

    const auto tMeshDesignVariables = DesignVariablesConversion{tMesh}.elementDensitiesToMeshDesignVariables(
        ElementDensityVectorReference{tElementDesignVariables});
    const auto tRoundTripElementDesignVariables =
        DesignVariablesConversion{tMesh}.meshDesignVariablesToElementDensityVector(tMeshDesignVariables);

    EXPECT_EQ(tElementDesignVariables, tRoundTripElementDesignVariables.mValue);
}

TEST_F(OneBlock3x1x1HexMesh, NodalDensitiesToNodalIDMap)
{
    const auto tMesh = DesignVariablesConversion{Mesh{mMeshFilePath, {}}};
    check_map_consistency(tMesh, mCommandGenerator.numberOfNodes(), TEST_CONTEXT("One block hex"));
}

TEST_F(TwoDThreeBlockMesh, NodalDensitiesToNodalIDMap)
{
    const auto tMesh = DesignVariablesConversion{Mesh{mMeshFilePath, {"block_1"}}};
    constexpr auto tNumberOfDesignVariables = 6U;
    check_map_consistency(tMesh, tNumberOfDesignVariables, TEST_CONTEXT("Three block, block 1 fixed"));
}

}  // namespace plato::mesh::unittest
