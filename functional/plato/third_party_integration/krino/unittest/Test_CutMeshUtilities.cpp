#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <iterator>
#include <map>
#include <stk_mesh/base/Types.hpp>
#include <string_view>
#include <vector>

#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/krino/BackgroundMesh.hpp"
#include "plato/third_party_integration/krino/BackgroundMeshUtilities.hpp"
#include "plato/third_party_integration/krino/CutMeshUtilities.hpp"
#include "plato/third_party_integration/krino/SnappingParameters.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/utilities/DataFilePath.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::third_party_integration::krino::unittest
{
namespace
{
using krino::test_utilities::KrinoTestFixture;

constexpr auto kWriteMeshName = std::string_view{"levelset_mesh.exo"};
constexpr auto kMeshName = std::string_view{"simple_mesh.exo"};

const auto kGoldCutMeshWithVoidNodes = std::map<std::size_t, double>{
    {1, -1.0}, {2, -1.0}, {3, -1.0}, {4, -1.0}, {5, 1.0},  {6, 1.0},  {7, 1.0},  {8, 1.0}, {9, 0.0},
    {10, 0.0}, {11, 0.0}, {12, 0.0}, {13, 0.0}, {14, 0.0}, {15, 0.0}, {16, 0.0}, {17, 0.0}};

const auto kOneTriMeshFilePath = utilities::data_file_path("one_tri.cdf");
const auto kFourTriTwoBlockMeshFilePath = utilities::data_file_path("four_tri_two_block.cdf");

constexpr double kOneQuarter{0.25};
constexpr double kThreeQuarters{0.75};

void create_mesh()
{
    const stk_io::CommandBounds tLowerBounds{0, 0, 0};
    const stk_io::CommandBounds tUpperBounds{1, 1, 1};
    const stk_io::CommandNumberOfElements tElements{1, 1, 1};

    stk_io::write_mesh(std::filesystem::path{kMeshName}, stk_io::CommandGenerator{tElements, tLowerBounds, tUpperBounds,
                                                                                  stk_io::CommandElementType::kTet});
}

void make_test_level_set_field(const BackgroundMesh& aBackgroundMesh)
{
    const auto tNodes = node_entities_in_mesh(aBackgroundMesh);

    std::vector<double> tVector(tNodes.size(), 1);
    auto tGenerator = [tSize = tNodes.size(), tIndex = 0UL]() mutable -> double
    { return (tIndex++ < tSize / 2) ? -1.0 : 1.0; };
    std::generate_n(tVector.begin(), tNodes.size(), tGenerator);

    for (const auto [tNode, tValue] : utilities::Zip(tNodes, tVector))
    {
        level_set_value(aBackgroundMesh.levelSetFields(), tNode) = tValue;
    }
}

[[nodiscard]] auto read_coordinates_and_level_sets(const std::filesystem::path& aFilePath)
{
    return stk_io::read_nodal_field(aFilePath, get_level_set_field_name());
}

void run_cut_and_write_test(const VoidPhase aVoidPhase,
                            const std::map<std::size_t, double>& aGold,
                            const plato::test_utilities::TestContext& aTestContext)
{
    create_mesh();
    const auto tBackgroundMesh = detail::initialize_background_mesh(kMeshName);
    make_test_level_set_field(tBackgroundMesh);
    cut_mesh(tBackgroundMesh, SnappingParameters{});
    const auto tSelector = create_output_selector(tBackgroundMesh.bulkData(), aVoidPhase);
    write_cut_mesh(tBackgroundMesh.bulkData(), kWriteMeshName, tSelector);

    const auto tCutLevelSetField = read_coordinates_and_level_sets(kWriteMeshName);
    EXPECT_EQ(tCutLevelSetField, aGold) << aTestContext;

    plato::test_utilities::test_for_existence_and_remove({kWriteMeshName}, TEST_CONTEXT("Write Mesh file"));
}
}  // namespace

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

TEST_F(KrinoTestFixture, CutMeshNodeIds)
{
    const auto tBackgroundMesh = test_utilities::background_mesh_from_vector(
        kOneTriMeshFilePath.value(), {kThreeQuarters, -kOneQuarter, -kOneQuarter});
    cut_mesh(tBackgroundMesh, SnappingParameters{});
    {
        const auto tResult = cut_mesh_node_ids(tBackgroundMesh.krinoMesh(), VoidPhase::kIncludeInMesh);
        const auto tGold = std::vector<stk::mesh::EntityId>{1, 2, 4, 5, 6};
        EXPECT_EQ(tGold, tResult) << "Include void.";
    }
    {
        const auto tResult = cut_mesh_node_ids(tBackgroundMesh.krinoMesh(), VoidPhase::kExcludeFromMesh);
        const auto tGold = std::vector<stk::mesh::EntityId>{1, 5, 6};
        EXPECT_EQ(tGold, tResult) << "Exclude void.";
    }
}

TEST_F(KrinoTestFixture, CutMeshNodeIdsFourTri)
{
    const auto tBackgroundMesh = test_utilities::background_mesh_from_vector(
        kFourTriTwoBlockMeshFilePath.value(),
        {kThreeQuarters, -kOneQuarter, -kOneQuarter, kThreeQuarters, kThreeQuarters, kThreeQuarters});
    cut_mesh(tBackgroundMesh, SnappingParameters{});
    {
        const auto tResult = cut_mesh_node_ids(tBackgroundMesh.krinoMesh(), VoidPhase::kIncludeInMesh);
        const auto tGold = std::vector<stk::mesh::EntityId>{1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        EXPECT_EQ(tGold, tResult) << "Include void.";
    }
    {
        const auto tResult = cut_mesh_node_ids(tBackgroundMesh.krinoMesh(), VoidPhase::kExcludeFromMesh);
        const auto tGold = std::vector<stk::mesh::EntityId>{1, 5, 6, 7, 8, 9, 10, 11, 12};
        EXPECT_EQ(tGold, tResult) << "Exclude void.";
    }
}

}  // namespace plato::third_party_integration::krino::unittest
