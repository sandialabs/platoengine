#include <gtest/gtest.h>

#include <algorithm>
#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/vector.hpp>
#include <iterator>
#include <unordered_map>

#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/krino/KrinoLevelSetPolicy.hpp"
#include "plato/third_party_integration/krino/SensitivityMapUtilities.hpp"
#include "plato/third_party_integration/krino/SnappingParameters.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/DataFilePath.hpp"
#include "plato/utilities/MPIUtilities.hpp"

namespace plato::third_party_integration::krino::parallel_unittest
{

class ParallelKrinoFixture : public krino::test_utilities::KrinoTestFixture
{
};

namespace
{

constexpr auto kNumRanks = int{4};
constexpr auto kWriteMeshName = std::string_view{"levelset_mesh.exo"};
const auto kTetBoxFilePath = utilities::data_file_path("box_3x4x7_tet4.cdf");
const auto kUnitSphere = Sphere{{0, 0, 0}, 1.0};
const auto kThreeQuarterOffsetXHatPlane = Plane{{-1, 0, 0}, 0.75};
const auto kFourTriTwoBlockMeshFilePath = utilities::data_file_path("four_tri_two_block.cdf");

void check_vector_same_on_all_ranks(const std::vector<stk::mesh::EntityId>& aVector,
                                    const plato::test_utilities::TestContext& aTestContext)
{
    const auto tCommunicator = boost::mpi::communicator{};
    const auto tRank = tCommunicator.rank();

    std::vector<stk::mesh::EntityId> tGoldVector;
    if (tRank == 0)
    {
        tGoldVector = aVector;
    }
    boost::mpi::broadcast(tCommunicator, tGoldVector, 0);

    EXPECT_EQ(tGoldVector, aVector) << aTestContext;
}

void remove_file_on_rank_zero()
{
    utilities::execute_on_root(boost::mpi::communicator{},
                               []()
                               {
                                   ::plato::test_utilities::test_for_existence_and_remove(
                                       {kWriteMeshName}, TEST_CONTEXT("Removing written cut mesh"));
                               });
}

}  // namespace

TEST(Krino, MPISize)
{
    const auto tComm = boost::mpi::communicator{};
    EXPECT_EQ(tComm.size(), kNumRanks);
}

TEST_F(ParallelKrinoFixture, InitializeKrino)
{
    ASSERT_TRUE(kTetBoxFilePath.has_value());
    const auto tKrinoMesh = read_and_setup_for_decomposition(kTetBoxFilePath.value());
    const auto tLevelSetField =
        make_level_set_field_from_primitives(LevelSetPrimitives{{}, {kUnitSphere}}, tKrinoMesh->bulk_data());
    cut_mesh(tKrinoMesh->bulk_data(), tLevelSetField, SnappingParameters{});
    const auto tSelector = create_output_selector(tKrinoMesh->bulk_data(), VoidPhase::kIncludeInMesh);
    write_mesh(tKrinoMesh->bulk_data(), kWriteMeshName, tSelector);
    remove_file_on_rank_zero();
}

TEST_F(ParallelKrinoFixture, CutMeshNodeIds)
{
    ASSERT_TRUE(kTetBoxFilePath.has_value());
    const auto tKrinoMesh = read_and_setup_for_decomposition(kTetBoxFilePath.value());
    const auto tLevelSetField =
        make_level_set_field_from_primitives(LevelSetPrimitives{{}, {kUnitSphere}}, tKrinoMesh->bulk_data());
    cut_mesh(tKrinoMesh->bulk_data(), tLevelSetField, SnappingParameters{});

    const auto tCutMeshNodeIds = cut_mesh_node_ids(*tKrinoMesh, VoidPhase::kExcludeFromMesh);

    check_vector_same_on_all_ranks(tCutMeshNodeIds, TEST_CONTEXT("Cut mesh node ids same on all ranks 3d sphere"));
}

namespace
{
void read_mesh_check_ids(const std::vector<stk::mesh::EntityId>& aCutMeshNodeIds)
{
    const auto tBulk = stk_io::read_mesh_bulk_data(std::string{kWriteMeshName});
    const auto& tParts = tBulk->mesh_meta_data().get_mesh_parts();
    auto tPartsVector = utilities::reserved_container<stk_io::PartReferenceVector>(tParts.size());
    std::ranges::transform(tParts, std::back_inserter(tPartsVector),
                           [](const auto aPart) { return std::cref(*aPart); });
    const auto tNodeIds = stk_io::node_ids(*tBulk, tPartsVector);
    EXPECT_EQ(tNodeIds, aCutMeshNodeIds);
}

void test_cut_mesh_node_ids_parallel_consistent(::krino::MeshInterface& aKrinoMesh,
                                                const VoidPhase aVoidPhase,
                                                const plato::test_utilities::TestContext& aTestContext)
{
    const auto tCutMeshNodeIds = cut_mesh_node_ids(aKrinoMesh, aVoidPhase);
    check_vector_same_on_all_ranks(tCutMeshNodeIds, aTestContext);
    const auto tSelector = create_output_selector(aKrinoMesh.bulk_data(), aVoidPhase);
    write_mesh(aKrinoMesh.bulk_data(), kWriteMeshName, tSelector);
    read_mesh_check_ids(tCutMeshNodeIds);
    remove_file_on_rank_zero();
}

}  // namespace

TEST_F(ParallelKrinoFixture, CutMeshNodeIdsFourTriParallelConsistent)
{
    ASSERT_TRUE(kFourTriTwoBlockMeshFilePath.has_value());
    const auto tKrinoMesh = read_and_setup_for_decomposition(kFourTriTwoBlockMeshFilePath.value());
    const auto tLevelSetField = make_level_set_field_from_primitives(
        LevelSetPrimitives{{kThreeQuarterOffsetXHatPlane}, {}}, tKrinoMesh->bulk_data());
    cut_mesh(tKrinoMesh->bulk_data(), tLevelSetField, SnappingParameters{});

    test_cut_mesh_node_ids_parallel_consistent(*tKrinoMesh, VoidPhase::kIncludeInMesh,
                                               TEST_CONTEXT("Cut nodes consistency using Include void region"));

    test_cut_mesh_node_ids_parallel_consistent(*tKrinoMesh, VoidPhase::kExcludeFromMesh,
                                               TEST_CONTEXT("Cut nodes consistency using Exclude void region"));
}

TEST_F(ParallelKrinoFixture, BackgroundMeshNodeIds)
{
    ASSERT_TRUE(kTetBoxFilePath.has_value());
    const auto tKrinoMesh = read_and_setup_for_decomposition(kTetBoxFilePath.value());
    const auto tLevelSetField =
        make_level_set_field_from_primitives(LevelSetPrimitives{{}, {kUnitSphere}}, tKrinoMesh->bulk_data());
    cut_mesh(tKrinoMesh->bulk_data(), tLevelSetField, SnappingParameters{});

    const auto tBackgroundNodeIds = background_node_ids(*tKrinoMesh, tLevelSetField);

    check_vector_same_on_all_ranks(tBackgroundNodeIds,
                                   TEST_CONTEXT("Background mesh node ids same on all ranks 3d sphere"));

    const auto tBulkData = stk_io::read_mesh_bulk_data(kTetBoxFilePath.value());
    const auto& tParts = tBulkData->mesh_meta_data().get_mesh_parts();
    const auto tDesignNodes = stk_io::node_ids(*tBulkData, *tParts.front());

    EXPECT_EQ(tDesignNodes, tBackgroundNodeIds);
}

TEST_F(ParallelKrinoFixture, BackgroundNodeIdsFourTri)
{
    test_utilities::four_tri_test_on_background_node_ids(TEST_CONTEXT("Four tri background nodes in parallel."));
}

TEST_F(ParallelKrinoFixture, GetLevelSetValues)
{
    ASSERT_TRUE(kFourTriTwoBlockMeshFilePath.has_value());
    const auto tKrinoMesh = read_and_setup_for_decomposition(kFourTriTwoBlockMeshFilePath.value());
    const auto tLevelSetField = make_level_set_field_from_primitives(
        LevelSetPrimitives{{kThreeQuarterOffsetXHatPlane}, {}}, tKrinoMesh->bulk_data());

    const auto tLevelSetValues = get_level_set_values(*tKrinoMesh, tLevelSetField);

    const auto tGold = std::unordered_map<stk::mesh::EntityId, double>{{1, .75}, {2, -.25}, {4, -0.25},
                                                                       {5, .75}, {6, 0.75}, {7, 0.75}};
    EXPECT_EQ(tLevelSetValues, tGold);
}

TEST_F(ParallelKrinoFixture, FixedBlock)
{
    ASSERT_TRUE(kFourTriTwoBlockMeshFilePath.has_value());
    const auto tKrinoMesh = read_and_setup_for_decomposition(kFourTriTwoBlockMeshFilePath.value(), {"block_2"});
    const auto tLevelSetField = make_level_set_field_from_primitives(
        LevelSetPrimitives{{kThreeQuarterOffsetXHatPlane}, {}}, tKrinoMesh->bulk_data());

    // Level-set values
    {
        const auto tLevelSetValues = get_level_set_values(*tKrinoMesh, tLevelSetField);
        const auto tExpected =
            std::unordered_map<stk::mesh::EntityId, double>{{1, .75}, {2, -.25}, {4, -0.25}, {7, 0.75}};
        EXPECT_EQ(tLevelSetValues, tExpected);
    }
    // Cut mesh node ids
    {
        cut_mesh(tKrinoMesh->bulk_data(), tLevelSetField, SnappingParameters{});
        const auto tCutMeshNodeIds = cut_mesh_node_ids(*tKrinoMesh, VoidPhase::kIncludeInMesh);
        constexpr auto tExpectedNumberOfCutMeshNodes = 9U;  // For both blocks, this would be 11
        EXPECT_EQ(tCutMeshNodeIds.size(), tExpectedNumberOfCutMeshNodes);
    }
    // Blocks
    {
        const auto tAllBlocks = all_block_parts(*tKrinoMesh);
        auto tBlockNames = utilities::reserved_container<std::vector<std::string>>(tAllBlocks.size());
        std::ranges::transform(tAllBlocks, std::back_inserter(tBlockNames),
                               [](const auto& aBlockPart) { return aBlockPart->name(); });

        const auto tExpectedBlockNames =
            std::vector<std::string>{"block_1", "block_2", "block_1_nonconformal", "block_1_void"};

        EXPECT_EQ(tBlockNames, tExpectedBlockNames);
    }
}

}  // namespace plato::third_party_integration::krino::parallel_unittest
