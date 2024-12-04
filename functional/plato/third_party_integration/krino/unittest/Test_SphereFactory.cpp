
#include <gtest/gtest.h>  // for AssertHelper, TEST, etc

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/krino/SphereFactory.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::third_party_integration::krino::unittest
{

namespace
{
constexpr common::Coordinate kZeroCoordinate{0, 0, 0};
constexpr common::Coordinate kTenCoordinate{10, 10, 10};

[[nodiscard]] SpherePatternData calculate_sphere_pattern_data_on_zero_ten_mesh(const double aSphereRadius,
                                                                               const double aSphereSpacing)
{
    return SpherePatternData{kZeroCoordinate, kTenCoordinate, aSphereRadius, aSphereSpacing};
}

void check_all_coordinates_equal_to(const common::Coordinate& aCoordinate,
                                    const double aExpected,
                                    const test_utilities::TestContext& aTestContext)
{
    EXPECT_DOUBLE_EQ(aCoordinate.x, aExpected) << aTestContext;
    EXPECT_DOUBLE_EQ(aCoordinate.y, aExpected) << aTestContext;
    EXPECT_DOUBLE_EQ(aCoordinate.z, aExpected) << aTestContext;
}

void check_spheres_equal(const Sphere& aSphere1,
                         const Sphere& aSphere2,
                         const test_utilities::TestContext& aTestContext)
{
    EXPECT_DOUBLE_EQ(aSphere1.mCenter.x, aSphere2.mCenter.x) << aTestContext;
    EXPECT_DOUBLE_EQ(aSphere1.mCenter.y, aSphere2.mCenter.y) << aTestContext;
    EXPECT_DOUBLE_EQ(aSphere1.mCenter.z, aSphere2.mCenter.z) << aTestContext;
    EXPECT_DOUBLE_EQ(aSphere1.mRadius, aSphere2.mRadius) << aTestContext;
}
}  // namespace

TEST(SphereFactory, GenerateSpheresCalculateSphereCenterCoords)
{
    constexpr int tNumValues{4};
    constexpr double tStart{-3.0};
    constexpr double tStep{15.0};
    const std::vector<double> tResult{
        detail::create_sphere_spacing_vector(tNumValues, detail::SphereStart{tStart}, detail::SphereStep{tStep})};
    const std::vector<double> tGold{-3, 12, 27, 42};
    EXPECT_EQ(tResult, tGold);
}

TEST(SphereFactory, CalculateNumSpheresAndStart_SphereJustInsideBBoxExtent)
{
    constexpr double tSphereRadius = 0.5;
    constexpr double tSphereSpacing = 4.5;
    const SpherePatternData tData{calculate_sphere_pattern_data_on_zero_ten_mesh(tSphereRadius, tSphereSpacing)};
    const common::Coordinate tNumSpheres{detail::calculate_num_spheres_in_each_direction(tData)};
    {
        constexpr auto tExpectedNumberOfSpheres = 3.0;
        check_all_coordinates_equal_to(tNumSpheres, tExpectedNumberOfSpheres, TEST_CONTEXT("Number of spheres"));
    }
    {
        const common::Coordinate tStart{detail::calculate_sphere_pattern_start(tNumSpheres, tData)};
        constexpr auto tExpectedStartCoordinate = 0.5;
        check_all_coordinates_equal_to(tStart, tExpectedStartCoordinate, TEST_CONTEXT("Sphere start coordinate"));
    }
}

TEST(SphereFactory, CalculateNumSpheresAndStart_SphereJustOutsideBBoxExtent)
{
    constexpr double tSphereRadius = 0.5;
    constexpr double tSphereSpacing = 5.5;
    const SpherePatternData tData{calculate_sphere_pattern_data_on_zero_ten_mesh(tSphereRadius, tSphereSpacing)};
    const common::Coordinate tNumSpheres{detail::calculate_num_spheres_in_each_direction(tData)};
    {
        constexpr auto tExpectedNumberOfSpheres = 3.0;
        check_all_coordinates_equal_to(tNumSpheres, tExpectedNumberOfSpheres,
                                       TEST_CONTEXT("Sphere just outside of bounding box"));
    }
    {
        const common::Coordinate tStart{detail::calculate_sphere_pattern_start(tNumSpheres, tData)};
        constexpr auto tExpectedStartCoordinate = -0.5;
        check_all_coordinates_equal_to(tStart, tExpectedStartCoordinate, TEST_CONTEXT("Sphere start coordinate"));
    }
}

TEST(SphereFactory, CalculateNumSpheresAndStart_SpheresNotIntersectingWithBoundary)
{
    constexpr double tSphereRadius = 0.1;
    constexpr double tSphereSpacing = 2.0;
    const SpherePatternData tData{calculate_sphere_pattern_data_on_zero_ten_mesh(tSphereRadius, tSphereSpacing)};
    const common::Coordinate tNumSpheres{detail::calculate_num_spheres_in_each_direction(tData)};
    {
        constexpr auto tExpectedNumberOfSpheres = 5.0;
        check_all_coordinates_equal_to(tNumSpheres, tExpectedNumberOfSpheres,
                                       TEST_CONTEXT("Spheres not intersecting with bounding box"));
    }
    {
        const common::Coordinate tStart{detail::calculate_sphere_pattern_start(tNumSpheres, tData)};
        constexpr auto tExpectedStartCoordinate = 1.0;
        check_all_coordinates_equal_to(tStart, tExpectedStartCoordinate, TEST_CONTEXT("Sphere start coordinate"));
    }
}

TEST(SphereFactory, CalculateNumSpheresAndStart_OneSphere)
{
    constexpr double tSphereRadius = 0.1;
    constexpr double tSphereSpacing = 100.0;
    const SpherePatternData tData{calculate_sphere_pattern_data_on_zero_ten_mesh(tSphereRadius, tSphereSpacing)};
    const common::Coordinate tNumSpheres{detail::calculate_num_spheres_in_each_direction(tData)};
    {
        constexpr auto tExpectedNumberOfSpheres = 1.0;
        check_all_coordinates_equal_to(tNumSpheres, tExpectedNumberOfSpheres, TEST_CONTEXT("One sphere"));
    }
    {
        const common::Coordinate tStart{detail::calculate_sphere_pattern_start(tNumSpheres, tData)};
        constexpr auto tExpectedStartCoordinate = 5.0;
        check_all_coordinates_equal_to(tStart, tExpectedStartCoordinate, TEST_CONTEXT("Sphere start coordinate"));
    }
}

TEST(SphereFactory, GenerateSpheres)
{
    constexpr double tSphereRadius = 0.25;
    constexpr double tSphereSpacing = 1.0;
    const common::Coordinate tBBoxMin{0, 0, 0};
    const common::Coordinate tBBoxMax{2, 1, 1};
    const SpherePatternData tData{tBBoxMin, tBBoxMax, tSphereRadius, tSphereSpacing};
    const std::vector<Sphere> tSpheres = generate_spheres(tData);

    const auto tExpectedSpheres =
        std::vector<Sphere>{{{0.0, 0.5, 0.5}, 0.25}, {{1.0, 0.5, 0.5}, 0.25}, {{2.0, 0.5, 0.5}, 0.25}};

    EXPECT_EQ(tSpheres.size(), tExpectedSpheres.size());

    for (const auto& [tComputedSphere, tExpectedSphere] : utilities::Zip{tSpheres, tExpectedSpheres})
    {
        check_spheres_equal(tComputedSphere, tExpectedSphere, TEST_CONTEXT("Generate spheres"));
    }
}

}  // namespace plato::third_party_integration::krino::unittest
