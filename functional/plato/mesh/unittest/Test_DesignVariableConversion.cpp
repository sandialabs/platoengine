#include <gtest/gtest.h>

#include <numeric>
#include <set>

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshBlocks.hpp"
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
using third_party_integration::stk_io::test_utilities::TwoDTwoBlockMesh;

void check_sizes(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                 const std::vector<unsigned int>& aExpectedEntitySizes,
                 const test_utilities::TestContext& aTestContext)
{
    EXPECT_EQ(aAnalysisDomainMesh.mBlockScalarField.size(), aExpectedEntitySizes.size()) << aTestContext;
    for (const auto& [tDesignVariables, tSize] :
         utilities::Zip{aAnalysisDomainMesh.mBlockScalarField, aExpectedEntitySizes})
    {
        EXPECT_EQ(tDesignVariables.second.size(), tSize) << aTestContext;
    }
}

void check_id_matches_density(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                              const test_utilities::TestContext& aTestContext)
{
    for (const auto& [tBlockID, tScalarField] : aAnalysisDomainMesh.mBlockScalarField)
    {
        for (const auto& tResult : tScalarField)
        {
            EXPECT_EQ(tResult.mValue, tResult.mGlobalMeshEntityID) << aTestContext;
        }
    }
}

void check_vector_index_matches_view_index(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                           const test_utilities::TestContext& aTestContext)
{
    const auto tDesignVariablesView = analysis::AnalysisDomainMeshSequentialView{aAnalysisDomainMesh};
    for (const auto& [tIndex, tFieldFromView] : utilities::enumerate(tDesignVariablesView))
    {
        const auto& tField = static_cast<analysis::ScalarFieldValue>(tFieldFromView);
        EXPECT_EQ(tIndex, tField.mDesignVariableVectorIndex) << aTestContext;
    }
}

std::vector<double> entity_ids_to_field(const std::vector<std::size_t>& aEntityIDs)
{
    auto tUniqueIDs = std::set<std::size_t>{};
    std::copy(aEntityIDs.cbegin(), aEntityIDs.cend(), std::inserter(tUniqueIDs, tUniqueIDs.begin()));
    auto tScalarField = std::vector<double>();
    tScalarField.reserve(tUniqueIDs.size());
    std::transform(tUniqueIDs.cbegin(), tUniqueIDs.cend(), std::back_inserter(tScalarField),
                   [](const std::size_t aIndex) { return static_cast<double>(aIndex); });
    return tScalarField;
}

template <typename MapFunction>
void check_map_consistency(const std::size_t aNumberOfDesignNodes,
                           const MapFunction& aMapFunction,
                           const test_utilities::TestContext& aTestContext)
{
    auto tNodalIDs = std::vector<std::size_t>(aNumberOfDesignNodes);
    std::iota(tNodalIDs.begin(), tNodalIDs.end(), 1);
    const auto tFieldMap = aMapFunction(entity_ids_to_field(tNodalIDs));
    for (const auto tNodalID : tNodalIDs)
    {
        const auto tNodalFieldMapIterator = tFieldMap.find(tNodalID);
        ASSERT_NE(tNodalFieldMapIterator, tFieldMap.end()) << aTestContext;
        EXPECT_EQ(tNodalFieldMapIterator->first, tNodalFieldMapIterator->second) << aTestContext;
    }
}

analysis::AnalysisDomainMesh one_block_mesh_analysis_for_tests(
    const third_party_integration::stk_io::CommandGenerator& aCommandGenerator,
    const std::filesystem::path& aMeshFilePath)
{
    auto tDesignVariables = std::vector<analysis::ScalarFieldValue>{};
    tDesignVariables.reserve(aCommandGenerator.numberOfNodes());
    std::generate_n(std::back_inserter(tDesignVariables), aCommandGenerator.numberOfNodes(),
                    [tCount = analysis::ScalarFieldValue::IndexType{0}]() mutable
                    {
                        ++tCount;
                        return analysis::ScalarFieldValue{tCount, tCount - 1, static_cast<double>(tCount)};
                    });
    return analysis::AnalysisDomainMesh{aMeshFilePath, {{0, tDesignVariables}}};
}

}  // namespace

TEST_F(TwoBlockMeshOnDisk, NodalScalarFieldToAnalysisDomainMesh)
{
    const auto tMesh = Mesh{mMeshFilePath};
    auto tScalarField = std::vector<double>(EntityCounts{tMesh}.numberOfNodes());
    constexpr auto tStartField = double{1.0};
    std::iota(tScalarField.begin(), tScalarField.end(), tStartField);
    const auto tAnalysisDomainMesh = DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        NodalFieldVectorReference{std::cref(tScalarField)});

    check_sizes(tAnalysisDomainMesh, {mExpectedNumberOfNodesInBlock1, mExpectedNumberOfNodesInBlock2},
                TEST_CONTEXT("Nodal design variables"));
    check_id_matches_density(tAnalysisDomainMesh, TEST_CONTEXT("Nodal densities"));
    check_vector_index_matches_view_index(tAnalysisDomainMesh, TEST_CONTEXT("Nodal design variables"));
}

TEST_F(TwoBlockMeshOnDisk, ElementScalarFieldToAnalysisDomainMesh)
{
    const auto tMesh = Mesh{mMeshFilePath};
    auto tScalarField = std::vector<double>(EntityCounts{tMesh}.numberOfElements());
    constexpr auto tStartField = double{1.0};
    std::iota(tScalarField.begin(), tScalarField.end(), tStartField);

    const auto tAnalysisDomainMesh =
        DesignVariablesConversion{tMesh}.elementFieldToAnalysisDomainMesh(ElementFieldVectorReference{tScalarField});

    check_sizes(tAnalysisDomainMesh, {mExpectedNumberOfElementsInBlock1, mExpectedNumberOfElementsInBlock2},
                TEST_CONTEXT("Element design variables"));
    check_id_matches_density(tAnalysisDomainMesh, TEST_CONTEXT("Element design variables"));
    check_vector_index_matches_view_index(tAnalysisDomainMesh, TEST_CONTEXT("Element design variables"));
}

TEST_F(TwoDThreeBlockMesh, NodeScalarFieldToDesignVariablesWithFixedBlocks)
{
    const auto tMesh = Mesh{mMeshFilePath, {mBlockNames[2]}};
    auto tNodeIds = MeshBlocks{tMesh}.nodeIDs(mBlock1Ordinal);
    const auto tNodeIdsBlock2 = MeshBlocks{tMesh}.nodeIDs(mBlock2Ordinal);
    tNodeIds.insert(tNodeIds.end(), tNodeIdsBlock2.cbegin(), tNodeIdsBlock2.cend());
    const auto tScalarField = entity_ids_to_field(tNodeIds);

    const auto tAnalysisDomainMesh =
        DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(NodalFieldVectorReference{tScalarField});

    check_sizes(tAnalysisDomainMesh, {mExpectedNumberOfNodesInBlock1, mExpectedNumberOfNodesInBlock2},
                TEST_CONTEXT("Nodal design variables"));
    check_id_matches_density(tAnalysisDomainMesh, TEST_CONTEXT("Nodal design variables"));
    check_vector_index_matches_view_index(tAnalysisDomainMesh, TEST_CONTEXT("Nodal design variables"));
}

TEST_F(TwoDThreeBlockMesh, ElementScalarFieldToDesignVariablesWithFixedBlocks)
{
    const auto tMesh = Mesh{mMeshFilePath, {mBlockNames[0], mBlockNames[1]}};
    const auto tElementIds = MeshBlocks{tMesh}.elementIDs(mBlock3Ordinal);
    const auto tScalarField = entity_ids_to_field(tElementIds);
    const auto tAnalysisDomainMesh =
        DesignVariablesConversion{tMesh}.elementFieldToAnalysisDomainMesh(ElementFieldVectorReference{tScalarField});

    check_sizes(tAnalysisDomainMesh, {mExpectedNumberOfElementsInBlock3}, TEST_CONTEXT("Element design variables"));
    check_id_matches_density(tAnalysisDomainMesh, TEST_CONTEXT("Element design variables"));
    check_vector_index_matches_view_index(tAnalysisDomainMesh, TEST_CONTEXT("Element design variables"));
}

TEST_F(OneBlock3x1x1HexMesh, AnalysisDomainMeshToNodalScalarField)
{
    const auto tAnalysisDomainMesh = one_block_mesh_analysis_for_tests(mCommandGenerator, mMeshFilePath);

    const auto tMesh = Mesh{mMeshFilePath, {}};
    const auto tDesignVariableVector =
        DesignVariablesConversion{tMesh}.analysisDomainMeshToNodalFieldVector(tAnalysisDomainMesh);

    auto tExpectedIndices = std::vector<analysis::ScalarFieldValue::IndexType>(mCommandGenerator.numberOfNodes());
    constexpr auto tStartIndex = analysis::ScalarFieldValue::IndexType{1};
    std::iota(tExpectedIndices.begin(), tExpectedIndices.end(), tStartIndex);

    EXPECT_EQ(tDesignVariableVector.mValue.size(), tExpectedIndices.size());
    for (const auto& [tResult, tExpected] : utilities::Zip{tDesignVariableVector.mValue, tExpectedIndices})
    {
        EXPECT_EQ(static_cast<double>(tExpected), tResult);
    }
}

TEST_F(TwoDThreeBlockMesh, AnalysisDomainMeshToElementScalarField)
{
    // Set up to assume blocks 1 and 2 are fixed
    constexpr auto tBlock3ElementGlobalID = analysis::ScalarFieldValue::IndexType{3};
    constexpr auto tVectorIndex = analysis::ScalarFieldValue::IndexType{0};
    constexpr auto tArbitraryField = double{0.5};
    const auto tDesignVariables =
        std::vector<analysis::ScalarFieldValue>{{tBlock3ElementGlobalID, tVectorIndex, tArbitraryField}};
    constexpr auto tBlock3ID = analysis::AnalysisDomainMesh::BlockIDType{3};
    const auto tAnalysisDomainMesh = analysis::AnalysisDomainMesh{mMeshFilePath, {{tBlock3ID, tDesignVariables}}};

    const auto tMesh = Mesh{mMeshFilePath, {mBlockNames[0], mBlockNames[1]}};
    const auto tResultDesignVariableVector =
        DesignVariablesConversion{tMesh}.analysisDomainMeshToElementFieldVector(tAnalysisDomainMesh);

    const auto tExpectedDesignVariableVector = std::vector{tArbitraryField};
    EXPECT_EQ(tResultDesignVariableVector.mValue, tExpectedDesignVariableVector);
}

TEST_F(TwoDThreeBlockMesh, MeshNodalDesignVariablesRoundTrip)
{
    const auto tMesh = Mesh{mMeshFilePath, {mBlockNames[2]}};
    auto tNodalDesignVariables = std::vector<double>(EntityCounts{tMesh}.numberOfDesignDomainNodes());
    constexpr auto tStartField = double{1.0};
    std::iota(tNodalDesignVariables.begin(), tNodalDesignVariables.end(), tStartField);

    const auto tAnalysisDomainMesh = DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        NodalFieldVectorReference{tNodalDesignVariables});
    const auto tRoundTripNodalDesignVariables =
        DesignVariablesConversion{tMesh}.analysisDomainMeshToNodalFieldVector(tAnalysisDomainMesh);

    EXPECT_EQ(tNodalDesignVariables, tRoundTripNodalDesignVariables.mValue);
}

TEST_F(TwoDThreeBlockMesh, MeshElementDesignVariablesRoundTrip)
{
    const auto tMesh = Mesh{mMeshFilePath, {mBlockNames[0]}};
    auto tElementDesignVariables = std::vector<double>(EntityCounts{tMesh}.numberOfDesignDomainElements());
    constexpr auto tStartField = double{1.0};
    std::iota(tElementDesignVariables.begin(), tElementDesignVariables.end(), tStartField);

    const auto tAnalysisDomainMesh = DesignVariablesConversion{tMesh}.elementFieldToAnalysisDomainMesh(
        ElementFieldVectorReference{tElementDesignVariables});
    const auto tRoundTripElementDesignVariables =
        DesignVariablesConversion{tMesh}.analysisDomainMeshToElementFieldVector(tAnalysisDomainMesh);

    EXPECT_EQ(tElementDesignVariables, tRoundTripElementDesignVariables.mValue);
}
}  // namespace plato::mesh::unittest
