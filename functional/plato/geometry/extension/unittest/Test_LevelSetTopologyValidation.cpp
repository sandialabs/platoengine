#include <gtest/gtest.h>

#include <filesystem>

#include "plato/geometry/extension/LevelSetTopology.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::geometry::extension::unittest
{
namespace
{
const auto kLevelSetTopology = plato::test_utilities::create_valid_level_set_topology_geometry();
}  // namespace

TEST(LevelsetTopologyValidation, ValidateOutputMeshName)
{
    auto tLevelSetTopology = kLevelSetTopology;
    EXPECT_FALSE(detail::validate_output_mesh_name(tLevelSetTopology).has_value());
    tLevelSetTopology.output_mesh_name = boost::none;
    EXPECT_TRUE(detail::validate_output_mesh_name(tLevelSetTopology).has_value());
}

TEST(LevelsetTopologyValidation, ValidateBackgroundMeshName)
{
    auto tLevelSetTopology = kLevelSetTopology;
    EXPECT_FALSE(detail::validate_background_mesh_name(tLevelSetTopology).has_value());
    tLevelSetTopology.background_mesh_name = boost::none;
    EXPECT_TRUE(detail::validate_background_mesh_name(tLevelSetTopology).has_value());
}

TEST(LevelsetTopologyValidation, ValidateCutMeshName)
{
    auto tLevelSetTopology = kLevelSetTopology;
    EXPECT_FALSE(detail::validate_cut_mesh_name(tLevelSetTopology).has_value());
    tLevelSetTopology.cut_mesh_name = boost::none;
    EXPECT_TRUE(detail::validate_cut_mesh_name(tLevelSetTopology).has_value());
}

TEST(LevelsetTopologyValidation, ValidateLowerBound)
{
    constexpr double tLargestAllowableValue = 0;
    constexpr double tDelta = 1e-6;
    auto tLevelSetTopology = kLevelSetTopology;
    tLevelSetTopology.levelset_lower_bound = tLargestAllowableValue + tDelta;
    EXPECT_TRUE(detail::validate_lower_bound(tLevelSetTopology).has_value());
    tLevelSetTopology.levelset_lower_bound = tLargestAllowableValue - tDelta;
    EXPECT_FALSE(detail::validate_lower_bound(tLevelSetTopology).has_value());
}

TEST(LevelsetTopologyValidation, ValidateUpperBound)
{
    constexpr double tSmallestAllowableValue = 0;
    constexpr double tDelta = 1e-6;
    auto tLevelSetTopology = kLevelSetTopology;
    tLevelSetTopology.levelset_upper_bound = tSmallestAllowableValue - tDelta;
    EXPECT_TRUE(detail::validate_upper_bound(tLevelSetTopology).has_value());
    tLevelSetTopology.levelset_upper_bound = tSmallestAllowableValue + tDelta;
    EXPECT_FALSE(detail::validate_upper_bound(tLevelSetTopology).has_value());
}

TEST(LevelsetTopologyValidation, ValidateSpherePatternSpacing)
{
    constexpr double tSmallestAllowableValue = 1e-5;
    constexpr double tDelta = 1e-6;
    auto tLevelSetTopology = kLevelSetTopology;
    tLevelSetTopology.sphere_pattern_spacing = tSmallestAllowableValue - tDelta;
    EXPECT_TRUE(detail::validate_sphere_pattern_spacing(tLevelSetTopology).has_value());
    tLevelSetTopology.sphere_pattern_spacing = tSmallestAllowableValue + tDelta;
    EXPECT_FALSE(detail::validate_sphere_pattern_spacing(tLevelSetTopology).has_value());
}

TEST(LevelsetTopologyValidation, ValidateSpherePatternRadius)
{
    constexpr double tSmallestAllowableValue = 1e-5;
    constexpr double tDelta = 1e-6;
    auto tLevelSetTopology = kLevelSetTopology;
    tLevelSetTopology.sphere_pattern_radius = tSmallestAllowableValue - tDelta;
    EXPECT_TRUE(detail::validate_sphere_pattern_radius(tLevelSetTopology).has_value());
    tLevelSetTopology.sphere_pattern_radius = tSmallestAllowableValue + tDelta;
    EXPECT_FALSE(detail::validate_sphere_pattern_radius(tLevelSetTopology).has_value());
}

TEST(LevelsetTopologyValidation, ValidateSpherePatternBoundingBox)
{
    auto tLevelSetTopology = kLevelSetTopology;
    tLevelSetTopology.sphere_pattern_bbox_max_x = 1.0;
    tLevelSetTopology.sphere_pattern_bbox_max_y = 1.0;
    tLevelSetTopology.sphere_pattern_bbox_max_z = 1.0;
    tLevelSetTopology.sphere_pattern_bbox_min_x = 0.0;
    tLevelSetTopology.sphere_pattern_bbox_min_y = 0.0;
    tLevelSetTopology.sphere_pattern_bbox_min_z = 0.0;
    EXPECT_FALSE(detail::validate_sphere_pattern_bbox(tLevelSetTopology).has_value());
    tLevelSetTopology.sphere_pattern_bbox_min_x = 2.0;
    EXPECT_TRUE(detail::validate_sphere_pattern_bbox(tLevelSetTopology).has_value());
    tLevelSetTopology.sphere_pattern_bbox_min_x = 0.0;
    tLevelSetTopology.sphere_pattern_bbox_min_y = 2.0;
    EXPECT_TRUE(detail::validate_sphere_pattern_bbox(tLevelSetTopology).has_value());
    tLevelSetTopology.sphere_pattern_bbox_min_y = 0.0;
    tLevelSetTopology.sphere_pattern_bbox_min_z = 2.0;
    EXPECT_TRUE(detail::validate_sphere_pattern_bbox(tLevelSetTopology).has_value());
}

}  // namespace plato::geometry::extension::unittest
