#include <gtest/gtest.h>

#include <filesystem>

#include "plato/geometry/extension/LevelsetTopology.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::geometry::extension::unittest
{
namespace
{
const auto kLevelsetTopology = plato::test_utilities::create_valid_levelset_topology_geometry();
}  // namespace

TEST(LevelsetTopologyValidation, ValidateOutputMeshName)
{
    auto tLevelsetTopology = kLevelsetTopology;
    EXPECT_FALSE(detail::validate_output_mesh_name(tLevelsetTopology).has_value());
    tLevelsetTopology.output_mesh_name = boost::none;
    EXPECT_TRUE(detail::validate_output_mesh_name(tLevelsetTopology).has_value());
}

TEST(LevelsetTopologyValidation, ValidateBackgroundMeshName)
{
    auto tLevelsetTopology = kLevelsetTopology;
    EXPECT_FALSE(detail::validate_background_mesh_name(tLevelsetTopology).has_value());
    tLevelsetTopology.background_mesh_name = boost::none;
    EXPECT_TRUE(detail::validate_background_mesh_name(tLevelsetTopology).has_value());
}

TEST(LevelsetTopologyValidation, ValidateCutMeshName)
{
    auto tLevelsetTopology = kLevelsetTopology;
    EXPECT_FALSE(detail::validate_cut_mesh_name(tLevelsetTopology).has_value());
    tLevelsetTopology.cut_mesh_name = boost::none;
    EXPECT_TRUE(detail::validate_cut_mesh_name(tLevelsetTopology).has_value());
}

TEST(LevelsetTopologyValidation, ValidateLowerBound)
{
    constexpr double tLargestAllowableValue = 0;
    constexpr double tDelta = 1e-6;
    auto tLevelsetTopology = kLevelsetTopology;
    tLevelsetTopology.levelset_lower_bound = tLargestAllowableValue + tDelta;
    EXPECT_TRUE(detail::validate_lower_bound(tLevelsetTopology).has_value());
    tLevelsetTopology.levelset_lower_bound = tLargestAllowableValue - tDelta;
    EXPECT_FALSE(detail::validate_lower_bound(tLevelsetTopology).has_value());
}

TEST(LevelsetTopologyValidation, ValidateUpperBound)
{
    constexpr double tSmallestAllowableValue = 0;
    constexpr double tDelta = 1e-6;
    auto tLevelsetTopology = kLevelsetTopology;
    tLevelsetTopology.levelset_upper_bound = tSmallestAllowableValue - tDelta;
    EXPECT_TRUE(detail::validate_upper_bound(tLevelsetTopology).has_value());
    tLevelsetTopology.levelset_upper_bound = tSmallestAllowableValue + tDelta;
    EXPECT_FALSE(detail::validate_upper_bound(tLevelsetTopology).has_value());
}

TEST(LevelsetTopologyValidation, ValidateSpherePatternSpacing)
{
    constexpr double tSmallestAllowableValue = 1e-5;
    constexpr double tDelta = 1e-6;
    auto tLevelsetTopology = kLevelsetTopology;
    tLevelsetTopology.sphere_pattern_spacing = tSmallestAllowableValue - tDelta;
    EXPECT_TRUE(detail::validate_sphere_pattern_spacing(tLevelsetTopology).has_value());
    tLevelsetTopology.sphere_pattern_spacing = tSmallestAllowableValue + tDelta;
    EXPECT_FALSE(detail::validate_sphere_pattern_spacing(tLevelsetTopology).has_value());
}

TEST(LevelsetTopologyValidation, ValidateSpherePatternRadius)
{
    constexpr double tSmallestAllowableValue = 1e-5;
    constexpr double tDelta = 1e-6;
    auto tLevelsetTopology = kLevelsetTopology;
    tLevelsetTopology.sphere_pattern_radius = tSmallestAllowableValue - tDelta;
    EXPECT_TRUE(detail::validate_sphere_pattern_radius(tLevelsetTopology).has_value());
    tLevelsetTopology.sphere_pattern_radius = tSmallestAllowableValue + tDelta;
    EXPECT_FALSE(detail::validate_sphere_pattern_radius(tLevelsetTopology).has_value());
}

TEST(LevelsetTopologyValidation, ValidateSpherePatternBoundingBox)
{
    auto tLevelsetTopology = kLevelsetTopology;
    tLevelsetTopology.sphere_pattern_bbox_max_x = 1.0;
    tLevelsetTopology.sphere_pattern_bbox_max_y = 1.0;
    tLevelsetTopology.sphere_pattern_bbox_max_z = 1.0;
    tLevelsetTopology.sphere_pattern_bbox_min_x = 0.0;
    tLevelsetTopology.sphere_pattern_bbox_min_y = 0.0;
    tLevelsetTopology.sphere_pattern_bbox_min_z = 0.0;
    EXPECT_FALSE(detail::validate_sphere_pattern_bbox(tLevelsetTopology).has_value());
    tLevelsetTopology.sphere_pattern_bbox_min_x = 2.0;
    EXPECT_TRUE(detail::validate_sphere_pattern_bbox(tLevelsetTopology).has_value());
    tLevelsetTopology.sphere_pattern_bbox_min_x = 0.0;
    tLevelsetTopology.sphere_pattern_bbox_min_y = 2.0;
    EXPECT_TRUE(detail::validate_sphere_pattern_bbox(tLevelsetTopology).has_value());
    tLevelsetTopology.sphere_pattern_bbox_min_y = 0.0;
    tLevelsetTopology.sphere_pattern_bbox_min_z = 2.0;
    EXPECT_TRUE(detail::validate_sphere_pattern_bbox(tLevelsetTopology).has_value());
}

}  // namespace plato::geometry::extension::unittest
