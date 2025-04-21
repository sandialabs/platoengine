#include <gtest/gtest.h>

#include <filesystem>

#include "plato/geometry/extension/LevelSetTopology.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::geometry::extension::unittest
{
namespace
{
const auto kLevelSetTopology = create_valid_level_set_topology_geometry_input();
const auto kLevelSetWithoutSpherePattern = create_valid_level_set_topology_geometry_initialize_from_field_input();

class LevelSetTopologyValidationTwoBlockFixture
    : public third_party_integration::stk_io::test_utilities::ThreeDTwoBlockTetMesh
{
};

}  // namespace

TEST(LevelSetTopologyValidation, ValidateOutputMeshName)
{
    auto tLevelSetTopology = kLevelSetTopology;
    EXPECT_FALSE(library::detail::validate_output_name(tLevelSetTopology).has_value());
    tLevelSetTopology.output_name = boost::none;
    EXPECT_TRUE(library::detail::validate_output_name(tLevelSetTopology).has_value());
}

TEST(LevelSetTopologyValidation, ValidateBackgroundMeshName)
{
    auto tLevelSetTopology = kLevelSetTopology;
    EXPECT_FALSE(library::detail::validate_mesh_name(tLevelSetTopology).has_value());
    tLevelSetTopology.mesh_name = boost::none;
    EXPECT_TRUE(library::detail::validate_mesh_name(tLevelSetTopology).has_value());
}

TEST(LevelSetTopologyValidation, ValidateLowerBound)
{
    constexpr double tLargestAllowableValue = 0;
    constexpr double tDelta = 1e-6;
    auto tLevelSetTopology = kLevelSetTopology;
    tLevelSetTopology.level_set_lower_bound = tLargestAllowableValue + tDelta;
    EXPECT_TRUE(detail::validate_lower_bound(tLevelSetTopology).has_value());
    tLevelSetTopology.level_set_lower_bound = tLargestAllowableValue - tDelta;
    EXPECT_FALSE(detail::validate_lower_bound(tLevelSetTopology).has_value());
}

TEST(LevelSetTopologyValidation, ValidateUpperBound)
{
    constexpr double tSmallestAllowableValue = 0;
    constexpr double tDelta = 1e-6;
    auto tLevelSetTopology = kLevelSetTopology;
    tLevelSetTopology.level_set_upper_bound = tSmallestAllowableValue - tDelta;
    EXPECT_TRUE(detail::validate_upper_bound(tLevelSetTopology).has_value());
    tLevelSetTopology.level_set_upper_bound = tSmallestAllowableValue + tDelta;
    EXPECT_FALSE(detail::validate_upper_bound(tLevelSetTopology).has_value());
}

TEST(LevelSetTopologyValidation, ValidateSpherePatternSpacing)
{
    constexpr double tSmallestAllowableValue = 1e-5;
    constexpr double tDelta = 1e-6;
    auto tLevelSetTopology = kLevelSetTopology;
    tLevelSetTopology.sphere_pattern_spacing = tSmallestAllowableValue - tDelta;
    EXPECT_TRUE(detail::validate_sphere_pattern_spacing(tLevelSetTopology).has_value());
    tLevelSetTopology.sphere_pattern_spacing = tSmallestAllowableValue + tDelta;
    EXPECT_FALSE(detail::validate_sphere_pattern_spacing(tLevelSetTopology).has_value());

    EXPECT_FALSE(detail::validate_sphere_pattern_spacing(kLevelSetWithoutSpherePattern).has_value());
}

TEST(LevelSetTopologyValidation, ValidateSpherePatternRadius)
{
    constexpr double tSmallestAllowableValue = 1e-5;
    constexpr double tDelta = 1e-6;
    auto tLevelSetTopology = kLevelSetTopology;
    tLevelSetTopology.sphere_pattern_radius = tSmallestAllowableValue - tDelta;
    EXPECT_TRUE(detail::validate_sphere_pattern_radius(tLevelSetTopology).has_value());
    tLevelSetTopology.sphere_pattern_radius = tSmallestAllowableValue + tDelta;
    EXPECT_FALSE(detail::validate_sphere_pattern_radius(tLevelSetTopology).has_value());

    EXPECT_FALSE(detail::validate_sphere_pattern_radius(kLevelSetWithoutSpherePattern).has_value());
}

TEST(LevelSetTopologyValidation, ValidateSpherePatternBoundingBox)
{
    auto tLevelSetTopology = kLevelSetTopology;
    EXPECT_FALSE(detail::validate_sphere_pattern_bbox(tLevelSetTopology).has_value());
    tLevelSetTopology.sphere_pattern_bbox_min_x = 2.0;
    EXPECT_TRUE(detail::validate_sphere_pattern_bbox(tLevelSetTopology).has_value());
    tLevelSetTopology.sphere_pattern_bbox_min_x = 0.0;
    tLevelSetTopology.sphere_pattern_bbox_min_y = 2.0;
    EXPECT_TRUE(detail::validate_sphere_pattern_bbox(tLevelSetTopology).has_value());
    tLevelSetTopology.sphere_pattern_bbox_min_y = 0.0;
    tLevelSetTopology.sphere_pattern_bbox_min_z = 2.0;
    EXPECT_TRUE(detail::validate_sphere_pattern_bbox(tLevelSetTopology).has_value());

    EXPECT_FALSE(detail::validate_sphere_pattern_bbox(kLevelSetWithoutSpherePattern).has_value());
}

TEST(LevelSetTopologyValidation, ValidateSpherePatternBoundingBoxMissingEntries)
{
    {
        auto tLevelSetTopology = kLevelSetTopology;
        EXPECT_FALSE(detail::validate_sphere_pattern_bbox(tLevelSetTopology).has_value());
    }
    {
        auto tLevelSetTopology = kLevelSetTopology;
        tLevelSetTopology.sphere_pattern_bbox_min_x = boost::none;
        EXPECT_TRUE(detail::validate_sphere_pattern_bbox(tLevelSetTopology).has_value());
    }
    {
        auto tLevelSetTopology = kLevelSetTopology;
        tLevelSetTopology.sphere_pattern_bbox_min_y = boost::none;
        EXPECT_TRUE(detail::validate_sphere_pattern_bbox(tLevelSetTopology).has_value());
    }
    {
        auto tLevelSetTopology = kLevelSetTopology;
        tLevelSetTopology.sphere_pattern_bbox_min_z = boost::none;
        EXPECT_TRUE(detail::validate_sphere_pattern_bbox(tLevelSetTopology).has_value());
    }
    {
        auto tLevelSetTopology = kLevelSetTopology;
        tLevelSetTopology.sphere_pattern_bbox_max_x = boost::none;
        EXPECT_TRUE(detail::validate_sphere_pattern_bbox(tLevelSetTopology).has_value());
    }
    {
        auto tLevelSetTopology = kLevelSetTopology;
        tLevelSetTopology.sphere_pattern_bbox_max_y = boost::none;
        EXPECT_TRUE(detail::validate_sphere_pattern_bbox(tLevelSetTopology).has_value());
    }
    {
        auto tLevelSetTopology = kLevelSetTopology;
        tLevelSetTopology.sphere_pattern_bbox_max_z = boost::none;
        EXPECT_TRUE(detail::validate_sphere_pattern_bbox(tLevelSetTopology).has_value());
    }
}

TEST(LevelSetTopologyValidation, ValidateExactlyOneInitialTopologySpecifier)
{
    {
        auto tLevelSetTopology = kLevelSetTopology;
        EXPECT_FALSE(detail::validate_exactly_one_initial_level_set_specifier(tLevelSetTopology).has_value());
        tLevelSetTopology.initial_field_name = input_parser::IdentifierString{"gaba-ghoul"};
        EXPECT_TRUE(detail::validate_exactly_one_initial_level_set_specifier(tLevelSetTopology).has_value());
    }
    {
        auto tLevelSetTopology = kLevelSetWithoutSpherePattern;
        EXPECT_FALSE(detail::validate_exactly_one_initial_level_set_specifier(tLevelSetTopology).has_value());
        tLevelSetTopology.initial_field_name = boost::none;
        EXPECT_TRUE(detail::validate_exactly_one_initial_level_set_specifier(tLevelSetTopology).has_value());
    }
}

TEST_F(LevelSetTopologyValidationTwoBlockFixture, FixedBlockValidation)
{
    auto tLevelSetInputBase = kLevelSetTopology;
    tLevelSetInputBase.background_mesh_name = input_parser::FileName{mMeshFilePath};

    const auto tCheckForErrors =
        [](const input_parser::level_set_topology& aLevelSetInput, const test_utilities::TestContext& aTestContext)
    {
        const auto tInput = input_parser::InputBlockWrapper{aLevelSetInput};
        const auto tValidationMessages = input_validation::validate(tInput, {});
        EXPECT_FALSE(tValidationMessages.empty()) << aTestContext;
    };

    {
        auto tLevelSetInput = tLevelSetInputBase;
        tLevelSetInput.fixed_blocks = input_parser::FixedBlockList{std::vector<std::string>{"block_1", "block_2"}};
        tCheckForErrors(tLevelSetInput, TEST_CONTEXT("No design domain"));
    }
    {
        auto tLevelSetInput = tLevelSetInputBase;
        tLevelSetInput.fixed_blocks = input_parser::FixedBlockList{std::vector<std::string>{"block_42"}};
        tCheckForErrors(tLevelSetInput, TEST_CONTEXT("Fixed block doesn't exist"));
    }
    {
        auto tLevelSetInput = tLevelSetInputBase;
        tLevelSetInput.fixed_blocks = input_parser::FixedBlockList{std::vector<std::string>{"block_2", "block_2"}};
        tCheckForErrors(tLevelSetInput, TEST_CONTEXT("Fixed blocks not unique."));
    }
}
}  // namespace plato::geometry::extension::unittest
