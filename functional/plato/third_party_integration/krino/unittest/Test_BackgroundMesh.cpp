#include <gtest/gtest.h>

#include <cstddef>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/krino/BackgroundMesh.hpp"
#include "plato/third_party_integration/krino/BackgroundMeshUtilities.hpp"
#include "plato/third_party_integration/krino/CutMeshUtilities.hpp"
#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::third_party_integration::krino::unittest
{
namespace
{
using krino::test_utilities::KrinoTestFixture;
const auto kFourTriTwoBlockMeshFilePath = utilities::data_file_path("four_tri_two_block.cdf");
constexpr auto kWriteMeshName = std::string_view{"levelset_mesh.exo"};
}  // namespace

TEST_F(KrinoTestFixture, BackgroundMesh_FieldNames)
{
    const auto tBackgroundMesh = detail::initialize_background_mesh(kFourTriTwoBlockMeshFilePath.value());

    const auto tSelector = create_output_selector(tBackgroundMesh.bulkData(), VoidPhase::kIncludeInMesh);
    write_cut_mesh(tBackgroundMesh.bulkData(), kWriteMeshName, tSelector);

    const auto tFieldNames = stk_io::nodal_field_names(kWriteMeshName);

    const auto tFieldNameGold = std::vector<std::string>{"coordinates", "LS", "LS_COPYFORSNAPPING"};
    EXPECT_EQ(tFieldNameGold, tFieldNames);
    plato::test_utilities::test_for_existence_and_remove({kWriteMeshName},
                                                         TEST_CONTEXT("Write mesh with krino fields"));
}

TEST_F(KrinoTestFixture, InitializeBackgroundMesh)
{
    const auto tBackgroundMesh = detail::initialize_background_mesh(kFourTriTwoBlockMeshFilePath.value());

    ASSERT_EQ(tBackgroundMesh.levelSetFields().size(), 1U);
    EXPECT_EQ(tBackgroundMesh.levelSetFields().front().name, get_level_set_field_name());

    const auto tLevelSetValues = get_level_set_values(tBackgroundMesh);
    for (const auto& [tNode, tLevelSetValue] : tLevelSetValues)
    {
        EXPECT_EQ(tLevelSetValue, 0);
    }
}

TEST_F(KrinoTestFixture, BackgroundMeshFromPrimitives)
{
    const auto tDownwardPlanePrimitives =
        LevelSetPrimitives{.mPlanes = {Plane{.mNormal = {0, -1, 0}, .mOffset = 0.5}}, .mSpheres = {}};
    const auto tBackgroundMesh =
        background_mesh_from_primitives(kFourTriTwoBlockMeshFilePath.value(), tDownwardPlanePrimitives);

    const auto tLevelSetValues = get_level_set_values(tBackgroundMesh);

    const auto tGoldLevelSetValues = std::unordered_map<std::size_t, double>{
        {1, 0.5}, {2, 0.5}, {4, -0.5}, {5, -0.5}, {6, -1.5}, {7, 1.5}};  // distances of nodes on the mesh from plane
    EXPECT_EQ(tLevelSetValues, tGoldLevelSetValues);
}

TEST_F(KrinoTestFixture, BackgroundMeshFromAnalysisDomainMesh)
{
    const auto tAnalysisDomainMesh =
        analysis::AnalysisDomainMesh{kFourTriTwoBlockMeshFilePath.value(),
                                     analysis::AnalysisDomainMesh::BlockScalarField{
                                         {1U, {{1U, 0U, 1.0}, {2U, 1U, -1.0}, {4U, 2U, -1.0}, {7U, 3U, 1.0}}}}};
    const auto tFixedBlocks = std::set<std::string>{"block_2"};

    const auto tBackgroundMesh = background_mesh_from_analysis_domain_mesh(tAnalysisDomainMesh, tFixedBlocks);

    const auto tLevelSetValues = get_level_set_values(tBackgroundMesh);

    const auto tGoldLevelSetValues = std::unordered_map<std::size_t, double>{
        {1, 1.0}, {2, -1.0}, {4, -1.0}, {7, 1.0}};  // distances of nodes on the mesh from plane
    EXPECT_EQ(tLevelSetValues, tGoldLevelSetValues);
}
}  // namespace plato::third_party_integration::krino::unittest
