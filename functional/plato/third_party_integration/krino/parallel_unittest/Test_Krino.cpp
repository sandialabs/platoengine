#include <gtest/gtest.h>

#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/vector.hpp>
#include <iterator>

#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/third_party_integration/krino/SensitivityMapUtilities.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::third_party_integration::krino::parallel_unittest
{

using krino::test_utilities::KrinoTestFixture;
namespace
{

constexpr auto kNumRanks = int{4};
constexpr auto kWriteMeshName = std::string_view{"levelset_mesh.exo"};
const auto kTetBoxFilePath = utilities::data_file_path("box_3x4x7_tet4.cdf");
const auto kUnitSphere = Sphere{{0, 0, 0}, 1.0};
const auto kThreeQuarterOffsetXHatPlane = Plane{{-1, 0, 0}, 0.75};
const auto kFourTriTwoBlockMeshFilePath = utilities::data_file_path("four_tri_two_block.cdf");

void check_vector_same_on_all_ranks(const std::vector<stk::mesh::EntityId>& aVector)
{
    const auto tCommunicator = boost::mpi::communicator{};
    const auto tRank = tCommunicator.rank();

    std::vector<stk::mesh::EntityId> tGoldVector;
    if (tRank == 0)
    {
        tGoldVector = aVector;
    }
    boost::mpi::broadcast(tCommunicator, tGoldVector, 0);

    EXPECT_EQ(tGoldVector, aVector);
}

}  // namespace

TEST(Krino, MPISize)
{
    const auto tComm = boost::mpi::communicator{};
    EXPECT_EQ(tComm.size(), kNumRanks);
}

TEST_F(KrinoTestFixture, InitializeKrino)
{
    ASSERT_TRUE(kTetBoxFilePath.has_value());
    const auto tKrinoMesh = read_and_setup_for_decomposition(kTetBoxFilePath.value());
    const auto tLevelSetField =
        make_level_set_field_from_primitives(LevelSetPrimitives{{}, {kUnitSphere}}, tKrinoMesh->bulk_data());
    cut_mesh(tKrinoMesh->bulk_data(), tLevelSetField);
    write_mesh(tKrinoMesh->bulk_data(), kWriteMeshName, VoidPhase::kIncludeInMesh);

    if (boost::mpi::communicator{}.rank() == 0)
    {
        ::plato::test_utilities::test_for_existence_and_remove({kWriteMeshName},
                                                               TEST_CONTEXT("Checking for recombined file."));
    }
}

TEST_F(KrinoTestFixture, CutMeshNodeIds)
{
    ASSERT_TRUE(kTetBoxFilePath.has_value());
    const auto tKrinoMesh = read_and_setup_for_decomposition(kTetBoxFilePath.value());
    const auto tLevelSetField =
        make_level_set_field_from_primitives(LevelSetPrimitives{{}, {kUnitSphere}}, tKrinoMesh->bulk_data());
    cut_mesh(tKrinoMesh->bulk_data(), tLevelSetField);

    const auto tCutMeshNodeIds = cut_mesh_node_ids(*tKrinoMesh, VoidPhase::kExcludeFromMesh);

    check_vector_same_on_all_ranks(tCutMeshNodeIds);
}

namespace
{
void read_mesh_check_ids(const std::vector<stk::mesh::EntityId>& aCutMeshNodeIds)
{
    const auto tBulk = stk_io::read_mesh_bulk_data(std::string{kWriteMeshName});
    const auto& tParts = tBulk->mesh_meta_data().get_mesh_parts();
    stk_io::PartReferenceVector tPartsVector;
    tPartsVector.reserve(tParts.size());
    std::transform(tParts.begin(), tParts.end(), std::back_inserter(tPartsVector),
                   [](const auto aPart) { return std::cref(*aPart); });
    const auto tNodeIds = stk_io::node_ids(*tBulk, tPartsVector);
    EXPECT_EQ(tNodeIds, aCutMeshNodeIds);
}

void remove_file_on_rank_zero()
{
    boost::mpi::communicator{}.barrier();
    if (boost::mpi::communicator{}.rank() == 0)
    {
        ::plato::test_utilities::test_for_existence_and_remove({kWriteMeshName},
                                                               TEST_CONTEXT("Removing written cut mesh"));
    }
}

}  // namespace

TEST_F(KrinoTestFixture, CutMeshNodeIdsFourTriParallelConsistent)
{
    ASSERT_TRUE(kFourTriTwoBlockMeshFilePath.has_value());
    const auto tKrinoMesh = read_and_setup_for_decomposition(kFourTriTwoBlockMeshFilePath.value());
    const auto tLevelSetField = make_level_set_field_from_primitives(
        LevelSetPrimitives{{kThreeQuarterOffsetXHatPlane}, {}}, tKrinoMesh->bulk_data());
    cut_mesh(tKrinoMesh->bulk_data(), tLevelSetField);

    {
        const auto tCutMeshNodeIds = cut_mesh_node_ids(*tKrinoMesh, VoidPhase::kIncludeInMesh);
        check_vector_same_on_all_ranks(tCutMeshNodeIds);
        write_mesh(tKrinoMesh->bulk_data(), kWriteMeshName, VoidPhase::kIncludeInMesh);
        read_mesh_check_ids(tCutMeshNodeIds);
        remove_file_on_rank_zero();
    }
    {
        const auto tCutMeshNodeIds = cut_mesh_node_ids(*tKrinoMesh, VoidPhase::kExcludeFromMesh);
        check_vector_same_on_all_ranks(tCutMeshNodeIds);
        write_mesh(tKrinoMesh->bulk_data(), kWriteMeshName, VoidPhase::kExcludeFromMesh);
        read_mesh_check_ids(tCutMeshNodeIds);
        remove_file_on_rank_zero();
    }
}

TEST_F(KrinoTestFixture, BackgroundMeshNodeIds)
{
    ASSERT_TRUE(kTetBoxFilePath.has_value());
    const auto tKrinoMesh = read_and_setup_for_decomposition(kTetBoxFilePath.value());
    const auto tLevelSetField =
        make_level_set_field_from_primitives(LevelSetPrimitives{{}, {kUnitSphere}}, tKrinoMesh->bulk_data());
    cut_mesh(tKrinoMesh->bulk_data(), tLevelSetField);

    const auto tBackgroundNodeIds = background_node_ids(*tKrinoMesh, tLevelSetField);

    check_vector_same_on_all_ranks(tBackgroundNodeIds);

    const auto tBulkData = stk_io::read_mesh_bulk_data(kTetBoxFilePath.value());
    const auto& tParts = tBulkData->mesh_meta_data().get_mesh_parts();
    const auto tDesignNodes = stk_io::node_ids(*tBulkData, *tParts.front());

    EXPECT_EQ(tDesignNodes, tBackgroundNodeIds);
}

TEST_F(KrinoTestFixture, BackgroundNodeIdsFourTri)
{
    const auto tMesh = read_and_setup_for_decomposition(kFourTriTwoBlockMeshFilePath.value());
    const auto tLevelSetField =
        test_utilities::make_level_set_field_from_vector(*tMesh, {.75, -.25, -.25, 0.75, 0.75, 0.75});
    cut_mesh(tMesh->bulk_data(), tLevelSetField);

    const auto tResult = background_node_ids(*tMesh, tLevelSetField);
    const auto tGold = std::vector<stk::mesh::EntityId>{1, 2, 4, 5, 6, 7};
    EXPECT_EQ(tGold, tResult) << "Background node ids the same as serial run.";
}

}  // namespace plato::third_party_integration::krino::parallel_unittest
