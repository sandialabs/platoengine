#include <gtest/gtest.h>

#include <Akri_Phase_Support.hpp>
#include <algorithm>
#include <filesystem>
#include <string_view>

#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/krino/SnappingParameters.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/utilities/DataFilePath.hpp"
#include "plato/utilities/Enumerate.hpp"

namespace plato::third_party_integration::krino::unittest
{

namespace
{
using krino::test_utilities::KrinoTestFixture;
constexpr auto kMeshName = std::string_view{"simple_mesh.exo"};
constexpr auto kWriteMeshName = std::string_view{"levelset_mesh.exo"};
const auto kOneTriMeshFilePath = utilities::data_file_path("one_tri.cdf");
const auto kFourTriTwoBlockMeshFilePath = utilities::data_file_path("four_tri_two_block.cdf");

const auto kGoldCutMeshWithVoidNodes = std::map<std::size_t, double>{
    {1, -1.0}, {2, -1.0}, {3, -1.0}, {4, -1.0}, {5, 1.0},  {6, 1.0},  {7, 1.0},  {8, 1.0}, {9, 0.0},
    {10, 0.0}, {11, 0.0}, {12, 0.0}, {13, 0.0}, {14, 0.0}, {15, 0.0}, {16, 0.0}, {17, 0.0}};

void create_mesh()
{
    const stk_io::CommandBounds tLowerBounds{0, 0, 0};
    const stk_io::CommandBounds tUpperBounds{1, 1, 1};
    const stk_io::CommandNumberOfElements tElements{1, 1, 1};

    stk_io::write_mesh(std::filesystem::path{kMeshName}, stk_io::CommandGenerator{tElements, tLowerBounds, tUpperBounds,
                                                                                  stk_io::CommandElementType::Tet});
}

[[nodiscard]] auto make_test_level_set_field(::krino::MeshInterface& aKrinoMesh) -> std::vector<::krino::LS_Field>
{
    std::vector<::krino::LS_Field> tField = ::krino::Phase_Support::get_levelset_fields(aKrinoMesh.meta_data());
    const auto tNodes = node_entities_in_mesh(aKrinoMesh, tField);

    std::vector<double> tVector(tNodes.size(), 1);
    auto tGenerator = [N = tNodes.size(), i = 0UL]() mutable -> double { return (i++ < N / 2) ? -1.0 : 1.0; };
    std::generate_n(tVector.begin(), tNodes.size(), tGenerator);

    for (const auto [tNode, tValue] : utilities::Zip(tNodes, tVector))
    {
        level_set_value(tField, tNode) = tValue;
    }
    return tField;
}

[[nodiscard]] auto read_coordinates_and_level_sets(const std::filesystem::path& aFilePath)
{
    return stk_io::read_nodal_field(aFilePath, get_level_set_field_name());
}

void run_cut_and_write_test(const VoidPhase& aVoidPhase,
                            const std::map<std::size_t, double>& aGold,
                            const plato::test_utilities::TestContext& aTestContext)
{
    create_mesh();
    const auto tMesh = read_and_setup_for_decomposition(kMeshName);
    const auto tLevelSetField = make_test_level_set_field(*tMesh);
    cut_mesh(tMesh->bulk_data(), tLevelSetField, SnappingParameters{});
    write_mesh(tMesh->bulk_data(), kWriteMeshName, aVoidPhase);

    const auto tCutLevelSetField = read_coordinates_and_level_sets(kWriteMeshName);
    EXPECT_EQ(tCutLevelSetField, aGold) << aTestContext;

    plato::test_utilities::test_for_existence_and_remove({kWriteMeshName}, TEST_CONTEXT("Write Mesh file"));
}

}  // namespace

TEST_F(KrinoTestFixture, ReadAndSetupForDecomposition)
{
    create_mesh();
    const auto tMesh = read_and_setup_for_decomposition(kMeshName);

    write_mesh(tMesh->bulk_data(), kWriteMeshName, VoidPhase::kIncludeInMesh);

    const auto tFieldNames = stk_io::nodal_field_names(kWriteMeshName);

    const auto tFieldNameGold = std::vector<std::string>{"coordinates",
                                                         "CDFEM_SNAP_DISPLACEMENTS",
                                                         "CDFEM_SNAP_DISPLACEMENTS_STKFS_O",
                                                         "CDFEM_UP_4_PARENT_NODE_IDS",
                                                         "CDFEM_UP_4_PARENT_NODE_WTS",
                                                         "DistanceCorrectionDenominator",
                                                         "DistanceCorrectionNumerator",
                                                         "LEVEL_SET",
                                                         "LEVEL_SET_STASH",
                                                         "LS"};
    EXPECT_EQ(tFieldNameGold, tFieldNames);
    std::filesystem::remove(kMeshName);
    plato::test_utilities::test_for_existence_and_remove({kWriteMeshName}, TEST_CONTEXT("Write Mesh file"));
}

TEST_F(KrinoTestFixture, CutMeshIncludeVoid)
{
    run_cut_and_write_test(VoidPhase::kIncludeInMesh, kGoldCutMeshWithVoidNodes, TEST_CONTEXT("Void Included"));
}

TEST_F(KrinoTestFixture, CutMeshExcludeVoid)
{
    auto tGold = kGoldCutMeshWithVoidNodes;
    tGold.erase(tGold.find(1), std::next(tGold.find(4)));

    run_cut_and_write_test(VoidPhase::kExcludeFromMesh, tGold, TEST_CONTEXT("Void not Included"));
}

TEST_F(KrinoTestFixture, BackgroundNodeIds)
{
    const auto tMesh = read_and_setup_for_decomposition(kOneTriMeshFilePath.value());
    const auto tLevelSetField = test_utilities::make_level_set_field_from_vector(*tMesh, {.75, -.25, -.25});
    cut_mesh(tMesh->bulk_data(), tLevelSetField, SnappingParameters{});

    const auto tResult = background_node_ids(*tMesh, tLevelSetField);
    const auto tGold = std::vector<stk::mesh::EntityId>{1, 2, 4};
    EXPECT_EQ(tGold, tResult);
}

TEST_F(KrinoTestFixture, BackgroundNodeIdsFourTri)
{
    test_utilities::four_tri_test_on_background_node_ids(TEST_CONTEXT("Four tri background nodes in serial."));
}

TEST_F(KrinoTestFixture, CutMeshNodeIds)
{
    const auto tMesh = read_and_setup_for_decomposition(kOneTriMeshFilePath.value());
    const auto tLevelSetField = test_utilities::make_level_set_field_from_vector(*tMesh, {.75, -.25, -.25});
    cut_mesh(tMesh->bulk_data(), tLevelSetField, SnappingParameters{});
    {
        const auto tResult = cut_mesh_node_ids(*tMesh, VoidPhase::kIncludeInMesh);
        const auto tGold = std::vector<stk::mesh::EntityId>{1, 2, 4, 5, 6};
        EXPECT_EQ(tGold, tResult) << "Include void.";
    }
    {
        const auto tResult = cut_mesh_node_ids(*tMesh, VoidPhase::kExcludeFromMesh);
        const auto tGold = std::vector<stk::mesh::EntityId>{1, 5, 6};
        EXPECT_EQ(tGold, tResult) << "Exclude void.";
    }
}

TEST_F(KrinoTestFixture, CutMeshNodeIdsFourTri)
{
    const auto tMesh = read_and_setup_for_decomposition(kFourTriTwoBlockMeshFilePath.value());
    const auto tLevelSetField =
        test_utilities::make_level_set_field_from_vector(*tMesh, {.75, -.25, -.25, 0.75, 0.75, 0.75});
    cut_mesh(tMesh->bulk_data(), tLevelSetField, SnappingParameters{});
    {
        const auto tResult = cut_mesh_node_ids(*tMesh, VoidPhase::kIncludeInMesh);
        const auto tGold = std::vector<stk::mesh::EntityId>{1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        EXPECT_EQ(tGold, tResult) << "Include void.";
    }
    {
        const auto tResult = cut_mesh_node_ids(*tMesh, VoidPhase::kExcludeFromMesh);
        const auto tGold = std::vector<stk::mesh::EntityId>{1, 5, 6, 7, 8, 9, 10, 11, 12};
        EXPECT_EQ(tGold, tResult) << "Exclude void.";
    }
}

TEST_F(KrinoTestFixture, GetLevelSetFields)
{
    create_mesh();
    const auto tMesh = read_and_setup_for_decomposition(kMeshName);
    const auto tLevelSetFields = get_level_set_fields(*tMesh);

    ASSERT_EQ(tLevelSetFields.size(), 1U);
    EXPECT_EQ(tLevelSetFields.front().name, get_level_set_field_name());
    EXPECT_EQ(tLevelSetFields.front().isoval, 0.0);

    std::filesystem::remove(kMeshName);
}

TEST_F(KrinoTestFixture, GetLevelSetValuesMakeFromVector)
{
    create_mesh();
    const auto tMesh = read_and_setup_for_decomposition(kMeshName);

    const auto tVector = std::vector<double>{1., 2, 3, 4, 5, 6, 7, 8};
    const auto tLevelSetFields = test_utilities::make_level_set_field_from_vector(*tMesh, tVector);
    const auto tLevelSetValues = get_level_set_values(*tMesh, tLevelSetFields);
    for (const auto& [tId, tValue] : tLevelSetValues)
    {
        EXPECT_EQ(tValue, tId);
    }

    std::filesystem::remove(kMeshName);
}

}  // namespace plato::third_party_integration::krino::unittest
