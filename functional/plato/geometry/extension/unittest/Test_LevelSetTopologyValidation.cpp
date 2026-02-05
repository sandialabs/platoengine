#include <gtest/gtest.h>

#include "plato/geometry/extension/LevelSetTopology.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::geometry::extension::unittest
{
namespace
{
const auto kLevelSetTopology = geometry::extension::test_utilities::create_valid_level_set_topology_geometry_input();
const auto kLevelSetWithoutSpherePattern =
    geometry::extension::test_utilities::create_valid_level_set_topology_geometry_initialize_from_field_input();

class LevelSetTopologyValidationTwoBlockFixture
    : public third_party_integration::stk_io::test_utilities::ThreeDTwoBlockTetMesh
{
};

const auto kSphereOne = input_parser::LevelSetSphere{1.1, input_parser::Point{1, 2, 3}};
const auto kSphereTwo = input_parser::LevelSetSphere{2.1, input_parser::Point{4, 5, 6}};
const auto kSphereList = input_parser::LevelSetSphereList{{kSphereOne, kSphereTwo}};

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
    tLevelSetTopology.level_set_bounds = input_parser::Bounds{tLargestAllowableValue + tDelta, 1};
    EXPECT_TRUE(detail::validate_lower_bound(tLevelSetTopology).has_value());
    tLevelSetTopology.level_set_bounds = input_parser::Bounds{tLargestAllowableValue - tDelta, 1};
    EXPECT_FALSE(detail::validate_lower_bound(tLevelSetTopology).has_value());
}

TEST(LevelSetTopologyValidation, ValidateUpperBound)
{
    constexpr double tSmallestAllowableValue = 0;
    constexpr double tDelta = 1e-6;
    auto tLevelSetTopology = kLevelSetTopology;
    tLevelSetTopology.level_set_bounds = input_parser::Bounds{-1, tSmallestAllowableValue - tDelta};
    EXPECT_TRUE(detail::validate_upper_bound(tLevelSetTopology).has_value());
    tLevelSetTopology.level_set_bounds = input_parser::Bounds{-1, tSmallestAllowableValue + tDelta};
    EXPECT_FALSE(detail::validate_upper_bound(tLevelSetTopology).has_value());
}

TEST(LevelSetTopologyValidation, ValidateMaxSnappingEdgeLength)
{
    auto tLevelSetTopology = kLevelSetTopology;
    tLevelSetTopology.max_edge_length_percentage_for_snapping = -0.1;
    EXPECT_TRUE(detail::validate_max_snapping_edge_length(tLevelSetTopology).has_value());
    tLevelSetTopology.max_edge_length_percentage_for_snapping = 1.1;
    EXPECT_TRUE(detail::validate_max_snapping_edge_length(tLevelSetTopology).has_value());
    tLevelSetTopology.max_edge_length_percentage_for_snapping = 0.15;
    EXPECT_FALSE(detail::validate_max_snapping_edge_length(tLevelSetTopology).has_value());
}

TEST(LevelSetTopologyValidation, ValidateSpherePatternSpacing)
{
    constexpr double tSmallestAllowableValue = 1e-5;
    constexpr double tDelta = 1e-6;
    auto tLevelSetTopology = kLevelSetTopology;
    tLevelSetTopology.sphere_pattern.value().spacing = tSmallestAllowableValue - tDelta;
    EXPECT_TRUE(detail::validate_sphere_pattern_spacing(tLevelSetTopology).has_value());
    tLevelSetTopology.sphere_pattern.value().spacing = tSmallestAllowableValue + tDelta;
    EXPECT_FALSE(detail::validate_sphere_pattern_spacing(tLevelSetTopology).has_value());

    EXPECT_FALSE(detail::validate_sphere_pattern_spacing(kLevelSetWithoutSpherePattern).has_value());
}

TEST(LevelSetTopologyValidation, ValidateSpherePatternSpacingGreaterThanTwiceRadius)
{
    auto tLevelSetTopology = kLevelSetTopology;
    tLevelSetTopology.sphere_pattern.value().spacing = tLevelSetTopology.sphere_pattern.value().radius * 0.8;
    EXPECT_TRUE(detail::validate_sphere_pattern_spacing_greater_than_twice_radius(tLevelSetTopology).has_value())
        << "Spacing way too small";
    tLevelSetTopology.sphere_pattern.value().spacing = tLevelSetTopology.sphere_pattern.value().radius * 2;
    EXPECT_FALSE(detail::validate_sphere_pattern_spacing_greater_than_twice_radius(tLevelSetTopology).has_value())
        << "Spacing right at 2x radius and is ok";
}

TEST(LevelSetTopologyValidation, ValidateSpherePatternRadius)
{
    constexpr double tSmallestAllowableValue = 1e-5;
    constexpr double tDelta = 1e-6;
    auto tLevelSetTopology = kLevelSetTopology;
    tLevelSetTopology.sphere_pattern.value().radius = tSmallestAllowableValue - tDelta;
    EXPECT_TRUE(detail::validate_sphere_pattern_radius(tLevelSetTopology).has_value());
    tLevelSetTopology.sphere_pattern.value().radius = tSmallestAllowableValue + tDelta;
    EXPECT_FALSE(detail::validate_sphere_pattern_radius(tLevelSetTopology).has_value());

    EXPECT_FALSE(detail::validate_sphere_pattern_radius(kLevelSetWithoutSpherePattern).has_value());
}

TEST(LevelSetTopologyValidation, ValidateSpherePatternBoundingBox)
{
    auto tLevelSetTopology = kLevelSetTopology;
    EXPECT_FALSE(detail::validate_sphere_pattern_bbox(tLevelSetTopology).has_value())
        << detail::validate_sphere_pattern_bbox(tLevelSetTopology).value();
    tLevelSetTopology.sphere_pattern.value().min.mX = 2.0;
    EXPECT_TRUE(detail::validate_sphere_pattern_bbox(tLevelSetTopology).has_value());
    tLevelSetTopology.sphere_pattern.value().min.mX = 0.0;
    tLevelSetTopology.sphere_pattern.value().min.mY = 2.0;
    EXPECT_TRUE(detail::validate_sphere_pattern_bbox(tLevelSetTopology).has_value());
    tLevelSetTopology.sphere_pattern.value().min.mY = 0.0;
    tLevelSetTopology.sphere_pattern.value().min.mZ = 2.0;
    EXPECT_TRUE(detail::validate_sphere_pattern_bbox(tLevelSetTopology).has_value());

    EXPECT_FALSE(detail::validate_sphere_pattern_bbox(kLevelSetWithoutSpherePattern).has_value());
}

TEST(LevelSetTopologyValidation, ValidateSphereListRadii)
{
    auto tLevelSetTopology = kLevelSetTopology;
    tLevelSetTopology.initial_field_name = boost::none;
    tLevelSetTopology.sphere_list = kSphereList;
    EXPECT_FALSE(detail::validate_sphere_list_radii(tLevelSetTopology).has_value())
        << detail::validate_sphere_list_radii(tLevelSetTopology).value();
    tLevelSetTopology.sphere_list.value().mList.front().radius = 0;
    EXPECT_TRUE(detail::validate_sphere_list_radii(tLevelSetTopology).has_value());
}

TEST(LevelSetTopologyValidation, ValidateExactlyOneInitialTopologySpecifier)
{
    {
        auto tLevelSetTopology = kLevelSetTopology;
        EXPECT_FALSE(detail::validate_exactly_one_initial_level_set_specifier(tLevelSetTopology).has_value());
        tLevelSetTopology.initial_field_name = input_parser::IdentifierString{"gaba-ghoul"};
        EXPECT_TRUE(detail::validate_exactly_one_initial_level_set_specifier(tLevelSetTopology).has_value());
        tLevelSetTopology.initial_field_name = boost::none;
        tLevelSetTopology.sphere_list = kSphereList;
        EXPECT_TRUE(detail::validate_exactly_one_initial_level_set_specifier(tLevelSetTopology).has_value());
    }
    {
        auto tLevelSetTopology = kLevelSetWithoutSpherePattern;
        EXPECT_FALSE(detail::validate_exactly_one_initial_level_set_specifier(tLevelSetTopology).has_value());
        tLevelSetTopology.initial_field_name = boost::none;
        EXPECT_TRUE(detail::validate_exactly_one_initial_level_set_specifier(tLevelSetTopology).has_value());
        tLevelSetTopology.sphere_list = kSphereList;
        EXPECT_FALSE(detail::validate_exactly_one_initial_level_set_specifier(tLevelSetTopology).has_value());
    }
}

TEST_F(LevelSetTopologyValidationTwoBlockFixture, FixedBlockValidation)
{
    auto tLevelSetInputBase = kLevelSetTopology;
    tLevelSetInputBase.mesh_name = input_parser::FileName{mMeshFilePath};

    const auto tCheckForErrors = [](const input_parser::level_set_topology& aLevelSetInput,
                                    const plato::test_utilities::TestContext& aTestContext)
    {
        const auto tInput = input_parser::InputBlockWrapper{aLevelSetInput};
        const auto tValidationMessages = input_validation::validate(tInput, {});
        EXPECT_FALSE(tValidationMessages.empty()) << aTestContext;
    };

    {
        auto tLevelSetInput = tLevelSetInputBase;
        tLevelSetInput.fixed_blocks = input_parser::BlockList{std::vector<std::string>{mBlockNames[0], mBlockNames[1]}};
        tCheckForErrors(tLevelSetInput, TEST_CONTEXT("No design domain"));
    }
    {
        auto tLevelSetInput = tLevelSetInputBase;
        tLevelSetInput.fixed_blocks = input_parser::BlockList{std::vector<std::string>{"block_42"}};
        tCheckForErrors(tLevelSetInput, TEST_CONTEXT("Fixed block doesn't exist"));
    }
    {
        auto tLevelSetInput = tLevelSetInputBase;
        tLevelSetInput.fixed_blocks = input_parser::BlockList{std::vector<std::string>{mBlockNames[1], mBlockNames[1]}};
        tCheckForErrors(tLevelSetInput, TEST_CONTEXT("Fixed blocks not unique."));
    }
}

TEST(LevelSetTopologyDetail, GenerateSpheresFromList)
{
    auto tLevelSetTopology = kLevelSetTopology;
    tLevelSetTopology.initial_field_name = boost::none;
    tLevelSetTopology.sphere_list = kSphereList;
    const auto tKrinoList = detail::generate_spheres_from_list(tLevelSetTopology);
    ASSERT_EQ(tKrinoList.size(), kSphereList.mList.size());
    for (const auto& [tKrinoSphere, tLevelSetSphere] : utilities::Zip(tKrinoList, kSphereList.mList))
    {
        EXPECT_EQ(tKrinoSphere.mRadius, tLevelSetSphere.radius);
        EXPECT_EQ(tKrinoSphere.mCenter.x, tLevelSetSphere.center.mX);
        EXPECT_EQ(tKrinoSphere.mCenter.y, tLevelSetSphere.center.mY);
        EXPECT_EQ(tKrinoSphere.mCenter.z, tLevelSetSphere.center.mZ);
    }
}
}  // namespace plato::geometry::extension::unittest
