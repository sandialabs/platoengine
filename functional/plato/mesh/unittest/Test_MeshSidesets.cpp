#include <gtest/gtest.h>

#include <ranges>

#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshSidesets.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/krino/SensitivityTriangle.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::mesh::unittest
{

TEST(MeshSidesets, SidesetTriangles)
{
    constexpr double tTolerance{1e-14};
    constexpr size_t tGoldNumTris{2};
    const third_party_integration::common::Vector3 tGoldNormal{.x = 0, .y = 0, .z = -1};
    const auto tMeshPath = plato::utilities::data_file_path("two_block_one_interior_sideset_tet.cdf");
    std::set<std::string> tFixedBlockNames{"block_1"};
    const auto tMesh = MeshSidesets{Mesh{tMeshPath.value(), tFixedBlockNames}};
    const std::string tSidesetName{"my_ss"};
    const std::vector<third_party_integration::krino::SensitivityTriangle> tTriangles =
        tMesh.sidesetTriangles(tSidesetName);
    EXPECT_EQ(tTriangles.size(), tGoldNumTris);
    for (const auto tCurTriIndex : std::views::iota(0u, tGoldNumTris))
    {
        const third_party_integration::common::Vector3 tNormal = tTriangles[tCurTriIndex].normal();
        third_party_integration::common::test_utilities::test_near_equality_of_components(
            tNormal, tGoldNormal, tTolerance, TEST_CONTEXT("Testing normals of sideset triangles."));
    }
}

}  // namespace plato::mesh::unittest
