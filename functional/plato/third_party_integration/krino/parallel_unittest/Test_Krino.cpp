#include <gtest/gtest.h>

#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/vector.hpp>

#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
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

TEST_F(KrinoTestFixture, BackgroundMeshNodeIds)
{
    ASSERT_TRUE(kTetBoxFilePath.has_value());
    const auto tKrinoMesh = read_and_setup_for_decomposition(kTetBoxFilePath.value());
    const auto tLevelSetField =
        make_level_set_field_from_primitives(LevelSetPrimitives{{}, {kUnitSphere}}, tKrinoMesh->bulk_data());
    cut_mesh(tKrinoMesh->bulk_data(), tLevelSetField);

    const auto tBackgroundNodeIds = background_node_ids(*tKrinoMesh, tLevelSetField);

    check_vector_same_on_all_ranks(tBackgroundNodeIds);
}

}  // namespace plato::third_party_integration::krino::parallel_unittest
