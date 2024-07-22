#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "plato/mesh/MeshProxy.hpp"
#include "plato/filter/library/HashGeneration.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::integration_tests::serial
{
TEST(HashGeneration, HashMesh)
{
    namespace stk_io = plato::third_party_integration::stk_io;

    constexpr std::string_view fileName = "test.exo";

    // initial mesh
    {
        const stk_io::CommandGenerator tCommandGenerator{
            {2, 2, 2}, {-1, -1, -1}, {1, 1, 1}, stk_io::CommandElementType::Tet};
        stk_io::write_mesh(fileName, stk_io::generate_mesh(tCommandGenerator));
    }

    const mesh::MeshProxy tMeshProxy{fileName, std::vector<double>(stk_io::read_mesh_node_size(fileName))};
    const auto tInitialHash = plato::filter::library::hash_mesh_coordinates(tMeshProxy);

    // reload mesh and rehash
    EXPECT_TRUE(tInitialHash == plato::filter::library::hash_mesh_coordinates(tMeshProxy));

    // change mesh
    {
        const stk_io::CommandGenerator tCommandGenerator{
            {3, 3, 2}, {-1, -1, -1}, {1, 1, 1}, stk_io::CommandElementType::Tet};
        stk_io::write_mesh(fileName, stk_io::generate_mesh(tCommandGenerator));
    }
    EXPECT_FALSE(tInitialHash == plato::filter::library::hash_mesh_coordinates(tMeshProxy));

    test_utilities::test_for_existence_and_remove({fileName}, TEST_CONTEXT("Removing temporary files."));
}
}  // namespace plato::integration_tests::serial