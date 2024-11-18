
#include <gtest/gtest.h>  // for AssertHelper, TEST, etc

#include <cstdio>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/krino/SphereBuilder.hpp"

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

void check_repeated_num_spheres(const common::Coordinate &aNumSpheres, const double aGoldValue)
{
    EXPECT_EQ(aNumSpheres.x, aGoldValue);
    EXPECT_EQ(aNumSpheres.y, aGoldValue);
    EXPECT_EQ(aNumSpheres.z, aGoldValue);
}
}  // namespace

TEST(SphereBuilder, GenerateSpheresCalculateSphereCenterCoords)
{
    constexpr int tNumValues{4};
    constexpr double tStart{-3.0};
    constexpr double tStep{15.0};
    const std::vector<double> tResult{
        calculate_sphere_center_coords(tNumValues, NamedSphereCenterCoord{tStart}, tStep)};
    const std::vector<double> tGold{-3, 12, 27, 42};
    EXPECT_EQ(tResult, tGold);
}

TEST(SphereBuilder, CalculateNumSpheresAndStart_SphereJustInsideBBoxExtent)
{
    constexpr double tSphereRadius = 0.5;
    constexpr double tSphereSpacing = 4.9;
    const SpherePatternData tData{calculate_sphere_pattern_data_on_zero_ten_mesh(tSphereRadius, tSphereSpacing)};
    const common::Coordinate tNumSpheres{calculate_num_spheres_in_each_direction(tData)};
    check_repeated_num_spheres(tNumSpheres, 3);
    const common::Coordinate tStart{calculate_sphere_pattern_start(tNumSpheres, tData)};
    const common::Coordinate tGold{0.1, 0.1, 0.1};
    constexpr double tTol{1e-10};
    EXPECT_NEAR(tStart.x, tGold.x, tTol);
    EXPECT_NEAR(tStart.y, tGold.y, tTol);
    EXPECT_NEAR(tStart.z, tGold.z, tTol);
}

TEST(SphereBuilder, CalculateNumSpheresAndStart_SphereJustOutsideBBoxExtent)
{
    constexpr double tSphereRadius = 0.5;
    constexpr double tSphereSpacing = 5.1;
    const SpherePatternData tData{calculate_sphere_pattern_data_on_zero_ten_mesh(tSphereRadius, tSphereSpacing)};
    const common::Coordinate tNumSpheres{calculate_num_spheres_in_each_direction(tData)};
    check_repeated_num_spheres(tNumSpheres, 3);
    const common::Coordinate tStart{calculate_sphere_pattern_start(tNumSpheres, tData)};
    const common::Coordinate tGold{-0.1, -0.1, -0.1};
    constexpr double tTol{1e-10};
    EXPECT_NEAR(tStart.x, tGold.x, tTol);
    EXPECT_NEAR(tStart.y, tGold.y, tTol);
    EXPECT_NEAR(tStart.z, tGold.z, tTol);
}

TEST(SphereBuilder, CalculateNumSpheresAndStart_SpheresNotIntersectingWithBoundary)
{
    constexpr double tSphereRadius = 0.1;
    constexpr double tSphereSpacing = 2.0;
    const SpherePatternData tData{calculate_sphere_pattern_data_on_zero_ten_mesh(tSphereRadius, tSphereSpacing)};
    const common::Coordinate tNumSpheres{calculate_num_spheres_in_each_direction(tData)};
    check_repeated_num_spheres(tNumSpheres, 5);
    const common::Coordinate tStart{calculate_sphere_pattern_start(tNumSpheres, tData)};
    const common::Coordinate tGold{1, 1, 1};
    constexpr double tTol{1e-10};
    EXPECT_NEAR(tStart.x, tGold.x, tTol);
    EXPECT_NEAR(tStart.y, tGold.y, tTol);
    EXPECT_NEAR(tStart.z, tGold.z, tTol);
}

TEST(SphereBuilder, CalculateNumSpheresAndStart_OneSphere)
{
    constexpr double tSphereRadius = 0.1;
    constexpr double tSphereSpacing = 100.0;
    const SpherePatternData tData{calculate_sphere_pattern_data_on_zero_ten_mesh(tSphereRadius, tSphereSpacing)};
    const common::Coordinate tNumSpheres{calculate_num_spheres_in_each_direction(tData)};
    check_repeated_num_spheres(tNumSpheres, 1);
    const common::Coordinate tStart{calculate_sphere_pattern_start(tNumSpheres, tData)};
    const common::Coordinate tGold{5, 5, 5};
    constexpr double tTol{1e-10};
    EXPECT_NEAR(tStart.x, tGold.x, tTol);
    EXPECT_NEAR(tStart.y, tGold.y, tTol);
    EXPECT_NEAR(tStart.z, tGold.z, tTol);
}

TEST(SphereBuilder, GenerateSpheres)
{
    constexpr double tSphereRadius = 0.25;
    constexpr double tSphereSpacing = 1.0;
    const common::Coordinate tBBoxMin{0, 0, 0};
    const common::Coordinate tBBoxMax{2, 1, 1};
    const SpherePatternData tData{tBBoxMin, tBBoxMax, tSphereRadius, tSphereSpacing};
    const std::vector<Sphere> tSpheres = generate_spheres(tData);

    EXPECT_FLOAT_EQ(tSpheres[0].mCenter.x, 0);
    EXPECT_FLOAT_EQ(tSpheres[0].mCenter.y, 0.5);
    EXPECT_FLOAT_EQ(tSpheres[0].mCenter.z, 0.5);
    EXPECT_FLOAT_EQ(tSpheres[0].mRadius, 0.25);

    EXPECT_FLOAT_EQ(tSpheres[1].mCenter.x, 1);
    EXPECT_FLOAT_EQ(tSpheres[1].mCenter.y, 0.5);
    EXPECT_FLOAT_EQ(tSpheres[1].mCenter.z, 0.5);
    EXPECT_FLOAT_EQ(tSpheres[1].mRadius, 0.25);

    EXPECT_FLOAT_EQ(tSpheres[2].mCenter.x, 2);
    EXPECT_FLOAT_EQ(tSpheres[2].mCenter.y, 0.5);
    EXPECT_FLOAT_EQ(tSpheres[2].mCenter.z, 0.5);
    EXPECT_FLOAT_EQ(tSpheres[2].mRadius, 0.25);
}

}  // namespace plato::third_party_integration::krino::unittest
