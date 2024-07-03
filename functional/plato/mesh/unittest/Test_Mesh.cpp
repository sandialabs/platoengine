#include <gtest/gtest.h>

#include <filesystem>
#include <string_view>

#include "plato/mesh/Mesh.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"

namespace plato::mesh::unittest
{

constexpr std::string_view kMeshName = "example.exo";

namespace
{
auto generate_mesh_on_disk_and_return_generator() -> third_party_integration::stk_io::CommandGenerator
{
    const auto tGenerator = third_party_integration::stk_io::CommandGenerator{{3, 1, 1}, {0, 0, 0}, {3, 1, 1}};
    third_party_integration::stk_io::write_mesh(kMeshName, tGenerator);
    return tGenerator;
}

void clean_up()
{
    EXPECT_TRUE(std::filesystem::exists(kMeshName));
    EXPECT_TRUE(std::filesystem::remove(kMeshName));
}

}  // namespace

TEST(Mesh, AccessorFunctions)
{
    const auto tGenerator = generate_mesh_on_disk_and_return_generator();
    const Mesh tMesh(kMeshName);
    const unsigned int tSpatialDimensions = 3;

    EXPECT_EQ(tMesh.numberOfElements(), tGenerator.numberOfElements());
    EXPECT_EQ(tMesh.numberOfNodes(), tGenerator.numberOfNodes());
    EXPECT_EQ(tMesh.spatialDimensions(), tSpatialDimensions);

    const auto tNodes = tMesh.flattenedNodalCoordinates();
    EXPECT_EQ(tNodes.size(), tMesh.numberOfNodes() * tMesh.spatialDimensions());

    clean_up();
}

TEST(Mesh, CreateFromGeneratorAndWrite)
{
    const auto tGenerator = third_party_integration::stk_io::CommandGenerator{{2, 2, 2}, {0, 0, 0}, {2, 2, 2}};
    const Mesh tMesh(tGenerator);
    EXPECT_EQ(tGenerator.numberOfElements(), tMesh.numberOfElements());
    tMesh.write_mesh(kMeshName);
    EXPECT_TRUE(std::filesystem::exists(kMeshName));
    EXPECT_TRUE(std::filesystem::remove(kMeshName));
}

}  // namespace plato::mesh::unittest
