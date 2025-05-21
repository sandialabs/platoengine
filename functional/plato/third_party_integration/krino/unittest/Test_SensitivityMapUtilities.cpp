#include <gtest/gtest.h>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/krino/SensitivityMapUtilities.hpp"
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
        mKrinoMesh = read_and_setup_for_decomposition(kOneTriMeshFilePath.value());
        mLevelSetFields = test_utilities::make_level_set_field_from_vector(*mKrinoMesh, {1, -1, -1});
        cut_mesh(mKrinoMesh->bulk_data(), mLevelSetFields);
    }

   protected:
    std::unique_ptr<::krino::MeshInterface> mKrinoMesh;
    std::vector<::krino::LS_Field> mLevelSetFields;
};

}  // namespace

TEST(SensitivityMapUtilities, OneTriMeshPath) { ASSERT_TRUE(kOneTriMeshFilePath.has_value()); }

TEST_F(SensitivityMapUtilitiesFixture, GetChildNodeStencils)
{
    initialize();
    const auto tChildNodeStencils = get_child_node_stencils(mKrinoMesh->bulk_data());

    const auto tGold = std::vector{std::vector{1U, 2U}, std::vector{3U, 1U}};

    for (const auto& [tStencil, tGoldEntry] : utilities::Zip(tChildNodeStencils, tGold))
    {
        EXPECT_EQ(tStencil.parentNodes.front(), tGoldEntry.front());
        EXPECT_EQ(tStencil.parentNodes.back(), tGoldEntry.back());
    }
}

TEST_F(SensitivityMapUtilitiesFixture, ParentNodeIdsFromParentNodes)
{
    initialize();
    const auto tChildNodeStencils = get_child_node_stencils(mKrinoMesh->bulk_data());

    const auto tSecondStencil = tChildNodeStencils.back();
    const auto tParentIds = parent_node_ids_from_parent_nodes(mKrinoMesh->bulk_data(), tSecondStencil.parentNodes);
    const auto tGold = std::vector<BackgroundMeshNodeId>{4U, 1U};
    EXPECT_EQ(tParentIds, tGold);
}

TEST_F(SensitivityMapUtilitiesFixture, CoordinatesLevelSets)
{
    initialize();
    const auto tChildNodeStencils = get_child_node_stencils(mKrinoMesh->bulk_data());
    const auto tSecondStencil = tChildNodeStencils.back();
    const auto tSpatialDimension = 2U;
    const ::krino::FieldRef tCoordsField = mKrinoMesh->bulk_data().mesh_meta_data().coordinate_field();
    auto tSensitivity = level_set_coordinate_sensitivity(CoordinateFieldReference{tCoordsField},
                                                         LevelSetFieldReference{mLevelSetFields.front().isovar},
                                                         tSecondStencil.parentNodes, tSpatialDimension);

    const auto tGold = std::vector<common::Vector3>{{0.25, 0.25, 0}, {0.25, 0.25, 0}};
    for (const auto& [tResult, tGoldEntry] : utilities::Zip(tSensitivity, tGold))
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
