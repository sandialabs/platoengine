#include <gtest/gtest.h>

#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>

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

    const auto tCutMeshNodeIds = cut_mesh_node_ids(*tKrinoMesh, VoidPhase::kExcludeFromMesh);
    std::cout << "On rank : " << boost::mpi::communicator{}.rank()
              << " cut mesh node id size: " << tCutMeshNodeIds.size() << std::endl;

    write_mesh(tKrinoMesh->bulk_data(), kWriteMeshName, VoidPhase::kIncludeInMesh);

    if (boost::mpi::communicator{}.rank() == 0)
    {
        ::plato::test_utilities::test_for_existence_and_remove({kWriteMeshName},
                                                               TEST_CONTEXT("Checking for recombined file."));
    }
}

}  // namespace plato::third_party_integration::krino::parallel_unittest
