#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/krino/BackgroundMesh.hpp"
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
    const auto tBackgroundMesh = background_mesh_with_zero_level_set_field(kFourTriTwoBlockMeshFilePath.value());

    const auto tSelector = create_output_selector(tBackgroundMesh.mKrinoMesh->bulk_data(), VoidPhase::kIncludeInMesh);
    write_mesh(tBackgroundMesh.mKrinoMesh->bulk_data(), kWriteMeshName, tSelector);

    const auto tFieldNames = stk_io::nodal_field_names(kWriteMeshName);

    const auto tFieldNameGold = std::vector<std::string>{"coordinates", "LS", "LS_COPYFORSNAPPING"};
    EXPECT_EQ(tFieldNameGold, tFieldNames);
    plato::test_utilities::test_for_existence_and_remove({kWriteMeshName},
                                                         TEST_CONTEXT("Write mesh with krino fields"));
}

TEST_F(KrinoTestFixture, BackgroundMeshWithZeroLevelSetField)
{
    const auto tBackgroundMesh = background_mesh_with_zero_level_set_field(kFourTriTwoBlockMeshFilePath.value());

    ASSERT_EQ(tBackgroundMesh.mLevelSetFields.size(), 1U);
    EXPECT_EQ(tBackgroundMesh.mLevelSetFields.front().name, get_level_set_field_name());

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
}  // namespace plato::third_party_integration::krino::unittest
