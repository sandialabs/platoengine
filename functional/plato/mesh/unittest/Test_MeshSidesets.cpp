#include <gtest/gtest.h>

#include <ranges>

#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshSidesets.hpp"
#include "plato/third_party_integration/krino/SensitivityTriangle.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"

namespace plato::mesh::unittest
{

TEST(MeshSidesets, SidesetTriangles)
{
    constexpr double tTolerance{1e-14};
    constexpr size_t tGoldNumTris{2};
    const third_party_integration::common::Vector3 tGoldNormal{.x = 0, .y = 0, .z = -1};
    const auto tMeshPath = std::filesystem::path{"temp_mesh_save.exo"};
    constexpr auto tMeshString = std::string_view{
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
        "|dimension:3|sideset:name=my_ss;data=7,2,11,2"};  // data=<tet_id>,<side_id>,<tet_id>,<side_id>...
    third_party_integration::stk_io::write_mesh(tMeshPath, tMeshString);
    std::set<std::string> tFixedBlockNames{"BLOCK_1"};
    const auto tMesh = MeshSidesets{Mesh{tMeshPath, tFixedBlockNames}};
    const std::string tSidesetName{"MY_SS"};
    const std::vector<third_party_integration::krino::SensitivityTriangle> tTriangles =
        tMesh.sidesetTriangles(tSidesetName);
    EXPECT_EQ(tTriangles.size(), tGoldNumTris);
    for (const auto tCurTriIndex : std::views::iota(0u, tGoldNumTris))
    {
        const third_party_integration::common::UnitVector3 tNormal = tTriangles[tCurTriIndex].normal();
        EXPECT_NEAR(tNormal.x, tGoldNormal.x, tTolerance);
        EXPECT_NEAR(tNormal.y, tGoldNormal.y, tTolerance);
        EXPECT_NEAR(tNormal.z, tGoldNormal.z, tTolerance);
    }
    std::filesystem::remove(tMeshPath);
}

}  // namespace plato::mesh::unittest
