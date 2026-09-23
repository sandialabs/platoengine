#include <gtest/gtest.h>

#include <stk_mesh/base/Types.hpp>
#include <unordered_map>
#include <vector>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/krino/BackgroundMesh.hpp"
#include "plato/third_party_integration/krino/CutMeshUtilities.hpp"
#include "plato/third_party_integration/krino/SensitivityMapUtilities.hpp"
#include "plato/third_party_integration/krino/SnappingParameters.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
#include "plato/utilities/DataFilePath.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::third_party_integration::krino::unittest
{

namespace
{

using krino::test_utilities::KrinoTestFixture;
const auto kOneTriMeshFilePath = utilities::data_file_path("one_tri.cdf");

class SensitivityMapUtilitiesFixture : public KrinoTestFixture
{
   public:
    void initialize()
    {
        mBackgroundMesh = test_utilities::background_mesh_from_vector(kOneTriMeshFilePath.value(), {1, -1, -1});
        cut_mesh(mBackgroundMesh, SnappingParameters{});
    }

    BackgroundMesh mBackgroundMesh;
};

}  // namespace

TEST(SensitivityMapUtilities, OneTriMeshPath) { ASSERT_TRUE(kOneTriMeshFilePath.has_value()); }

TEST_F(SensitivityMapUtilitiesFixture, ParentNodeIDsFromGetKrinoSensitivities)
{
    initialize();
    const auto tSensitvitiesFromKrino = get_krino_sensitivities(mBackgroundMesh);
    constexpr unsigned int tNumberOfInterfaceNodes{2U};
    ASSERT_EQ(tSensitvitiesFromKrino.size(), tNumberOfInterfaceNodes);

    const auto tGoldParentNodeIDs = std::vector{std::vector{1U, 2U}, std::vector{4U, 1U}};

    for (const auto& [tLevelSetShapeSensitivity, tGoldEntry] :
         utilities::Zip(tSensitvitiesFromKrino, tGoldParentNodeIDs))
    {
        EXPECT_EQ(tLevelSetShapeSensitivity.parentNodeIds.front(), tGoldEntry.front());
        EXPECT_EQ(tLevelSetShapeSensitivity.parentNodeIds.back(), tGoldEntry.back());
    }
}

TEST_F(SensitivityMapUtilitiesFixture, CoordinateLevelSetSensitivity)
{
    initialize();
    const auto tSensitvitiesFromKrino = get_krino_sensitivities(mBackgroundMesh);
    const auto& tSecondSensitivity = tSensitvitiesFromKrino.back();
    const auto tSpatialDimension = 2U;
    const auto tSensitivityVector = coordinate_level_set_sensitivity(tSecondSensitivity, tSpatialDimension);
    const auto tGold = std::vector<common::Vector3>{{0.25, 0.25, 0}, {0.25, 0.25, 0}};
    for (const auto& [tResult, tGoldEntry] : utilities::Zip(tSensitivityVector, tGold))
    {
        common::test_utilities::test_double_equality_of_components(
            tResult, tGoldEntry, TEST_CONTEXT("Coordinates level sets sensitivities"));
    }
}

TEST(SensitivityMapUtilitiesDetail, ComputeHistogram)
{
    const auto tCutMeshIds = std::vector<stk::mesh::EntityId>{1, 2, 2, 3, 3, 3, 5, 5, 5, 5, 5, 6, 9, 10, 10};
    const auto tHistogram = detail::compute_histogram(tCutMeshIds);
    const auto tGold = std::unordered_map<stk::mesh::EntityId, unsigned int>{{2, 2}, {3, 3}, {5, 5}, {10, 2}};
    EXPECT_EQ(tGold, tHistogram);
}
}  // namespace plato::third_party_integration::krino::unittest
