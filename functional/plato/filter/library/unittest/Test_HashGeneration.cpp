#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/filter/library/HashGeneration.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/IOUtilities.hpp"

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
        stk_io::write_mesh(fileName, tCommandGenerator);
    }

    const analysis::AnalysisDomainMesh tAnalysisDomainMesh{fileName, {}};
    const auto tInitialHash = plato::filter::library::hash_mesh_coordinates(tAnalysisDomainMesh);

    // reload mesh and rehash
    EXPECT_EQ(tInitialHash, plato::filter::library::hash_mesh_coordinates(tAnalysisDomainMesh));

    // change mesh
    {
        const stk_io::CommandGenerator tCommandGenerator{
            {3, 3, 2}, {-1, -1, -1}, {1, 1, 1}, stk_io::CommandElementType::Tet};
        stk_io::write_mesh(fileName, tCommandGenerator);
    }
    EXPECT_NE(tInitialHash, plato::filter::library::hash_mesh_coordinates(tAnalysisDomainMesh));

    test_utilities::test_for_existence_and_remove({fileName}, TEST_CONTEXT("Removing temporary files."));
}
}  // namespace plato::integration_tests::serial