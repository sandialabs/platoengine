#include <gtest/gtest.h>

#include <stk_io/FillMesh.hpp>
#include <stk_io/StkMeshIoBroker.hpp>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/SidesetUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::third_party_integration::stk_io::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMeshWithNodeSets;
using third_party_integration::stk_io::test_utilities::TwoBlockMeshOnDisk;
using third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;

constexpr auto kExpectedNumberOfElementsInBlock1 = 273u;
constexpr auto kExpectedNumberOfElementsInBlock2 = 40u;
constexpr auto kExpectedNumberOfNodesInBlock1 = 93u;
constexpr auto kExpectedNumberOfNodesInBlock2 = 90u;

}  // namespace

TEST(SidesetUtilities, GetSidesetTriangles)
{
    /*  "0,1,HEX_8,1,2,3,4,5,6,7,8|sideset:name=surface_0; data=1,1"*/
    const auto tMeshPath = std::filesystem::path{"temp_mesh_save.exo"};
    constexpr auto tMesh = std::string_view{
        "textmesh:"
        "0,1,TET_4,5,1,2,3,block_1\n"
        "0,2,TET_4,6,5,2,3,block_1\n"
        "0,3,TET_4,6,7,5,3,block_1\n"
        "0,4,TET_4,6,4,7,3,block_1\n"
        "0,5,TET_4,6,2,4,3,block_1\n"
        "0,6,TET_4,6,8,7,4,block_1\n"
        "0,7,TET_4,9,5,6,7,block_2\n"
        "0,8,TET_4,10,9,6,7,block_2\n"
        "0,9,TET_4,10,11,9,7,block_2\n"
        "0,10,TET_4,10,8,11,7,block_2\n"
        "0,11,TET_4,10,6,8,7,block_2\n"
        "0,12,TET_4,10,12,11,8,block_2\n"
        "|coordinates: 0,-1,-1,0,0,-1,1,-1,-1,1,0,-1,0,-1,1,0,0,1,1,-1,1,1,0,1,0,-1,3,0,0,3,1,-1,3,1,0,3"
        "|dimension:3|sideset:name=my_ss;data=1,1"};
    write_mesh(tMeshPath, tMesh);
    /*
        const auto tBulkData = read_mesh_bulk_data(tMeshPath);
        const auto& tParts = tBulkData->mesh_meta_data().get_mesh_parts();
        constexpr auto tExpectedNumberOfParts = 1U;
        ASSERT_EQ(tParts.size(), tExpectedNumberOfParts);

        const auto tResultIDs = node_ids(*tBulkData, *tParts.front());
        const auto tExpectedIDs = std::vector<std::size_t>{1, 2, 3, 4, 11, 12, 13, 14};

        EXPECT_EQ(tResultIDs, tExpectedIDs);

        std::filesystem::remove(tMeshPath);
        */
}

}  // namespace plato::third_party_integration::stk_io::unittest
