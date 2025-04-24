#include <gtest/gtest.h>

#include <filesystem>

#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/extension/LevelSetTopology.hpp"
#include "plato/geometry/extension/MeshValidationUtilities.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshWithFieldWriter.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::geometry::extension::unittest
{

namespace
{
[[nodiscard]] auto create_density_geometry_input(const std::filesystem::path& aPath, const std::string& aField)
    -> input_parser::density_topology
{
    auto tInput = test_utilities::create_valid_density_topology_geometry_input();
    tInput.mesh_name = input_parser::FileName{aPath};
    tInput.initial_field_name = input_parser::IdentifierString{aField};
    return tInput;
}

[[nodiscard]] auto create_level_set_geometry_input(const std::filesystem::path& aPath, const std::string& aField)
    -> input_parser::level_set_topology
{
    auto tInput = test_utilities::create_valid_level_set_topology_geometry_initialize_from_field_input();
    tInput.mesh_name = input_parser::FileName{aPath};
    tInput.initial_field_name = input_parser::IdentifierString{aField};
    return tInput;
}

}  // namespace

using NodalDensityMesh = third_party_integration::stk_io::test_utilities::MeshWithNodalDensities;

TEST_F(NodalDensityMesh, DensityMeshFromInput)
{
    const auto tInput = create_density_geometry_input(mMeshName, mFieldName);
    const auto tMesh = mesh_from_input(tInput);
    const auto tTotalNodes = mesh::EntityCounts{tMesh}.numberOfNodes();
    EXPECT_EQ(mGoldNumbering.size(), tTotalNodes);
}

TEST_F(NodalDensityMesh, LevelSetMeshFromInput)
{
    const auto tInput = create_level_set_geometry_input(mMeshName, mFieldName);
    const auto tMesh = mesh_from_input(tInput);
    const auto tTotalNodes = mesh::EntityCounts{tMesh}.numberOfNodes();
    EXPECT_EQ(mGoldNumbering.size(), tTotalNodes);
}

namespace
{
void check_gold_numbering(const std::map<std::size_t, double>& aGoldMap,
                          const std::vector<double>& aResult,
                          const plato::test_utilities::TestContext& aTestContext)
{
    ASSERT_EQ(aGoldMap.size(), aResult.size()) << aTestContext;
    for (const auto [tNumbering, tResult] : utilities::Zip(aGoldMap, aResult))
    {
        EXPECT_EQ(tNumbering.second, tResult) << aTestContext;
    }
}

}  // namespace

TEST_F(NodalDensityMesh, DensityInitialValueFromMesh)
{
    const auto tInput = create_density_geometry_input(mMeshName, mFieldName);
    const auto tInitialGuess = initial_field_from_mesh(tInput);

    check_gold_numbering(mGoldNumbering, tInitialGuess, TEST_CONTEXT("Density read initial value from mesh."));
}

TEST_F(NodalDensityMesh, LevelSetInitialValueFromMesh)
{
    const auto tInput = create_level_set_geometry_input(mMeshName, mFieldName);
    const auto tInitialGuess = initial_field_from_mesh(tInput);

    check_gold_numbering(mGoldNumbering, tInitialGuess, TEST_CONTEXT("Level Set read initial value from mesh."));
}

TEST_F(NodalDensityMesh, MeshFieldNamesForErrorMessage)
{
    const auto tInput = create_density_geometry_input(mMeshName, mFieldName);
    const auto tString = mesh_field_names_for_error_message(tInput);
    const auto tGoldString =
        std::string{"Field name must be one of the following defined on the nodes: coordinates, topology."};
    EXPECT_EQ(tGoldString, tString);
}

}  // namespace plato::geometry::extension::unittest
