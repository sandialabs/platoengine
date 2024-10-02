
#include <gtest/gtest.h>  // for AssertHelper, TEST, etc

#include <cstdio>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/krino/SphereBuilder.hpp"

namespace plato::third_party_integration::krino::unittest
{

class PlatoTestKrino : public ::testing::Test
{
    // Even though this is doing nothing it is
    // needed because some of the other unit testing
    // is using a derived class and googletest won't let
    // you mix TEST and TEST_F in a single executable.
};

TEST_F(PlatoTestKrino, SphereBuilderGenerateSpheresCalculateSphereCenterCoords)
{
    constexpr int tNumValues{4};
    constexpr double tStart{-3.0};
    constexpr double tStep{15.0};
    const std::vector<double> tResult{calculate_sphere_center_coords(tNumValues, tStart, tStep)};
    const std::vector<double> tGold{-3, 12, 27, 42};
    EXPECT_EQ(tResult, tGold);
}

TEST_F(PlatoTestKrino, SphereBuilderCalculateNumSpheresAndStart_SphereJustInsideBBoxExtent)
{
    constexpr double tSphereRadius = 0.5;
    constexpr double tSphereSpacing = 4.9;
    const common::Coordinate tBBoxMin{0.0, 0.0, 0.0};
    const common::Coordinate tBBoxMax{10.0, 10.0, 10.0};
    const SpherePatternData tData{tBBoxMin, tBBoxMax, tSphereRadius, tSphereSpacing};
    const common::Coordinate tNumSpheres{calculate_num_spheres_in_each_direction(tData)};
    EXPECT_EQ(tNumSpheres.x, 3);
    EXPECT_EQ(tNumSpheres.y, 3);
    EXPECT_EQ(tNumSpheres.z, 3);
    const common::Coordinate tStart{calculate_sphere_pattern_start(tNumSpheres, tData)};
    const Coordinate tGold{0.1, 0.1, 0.1};
    constexpr double tTol{1e-10};
    ASSERT_NEAR(tStart.x, tGold.x, tTol);
    ASSERT_NEAR(tStart.y, tGold.y, tTol);
    ASSERT_NEAR(tStart.z, tGold.z, tTol);
}

TEST_F(PlatoTestKrino, SphereBuilderCalculateNumSpheresAndStart_SphereJustOutsideBBoxExtent)
{
    constexpr double tSphereRadius = 0.5;
    constexpr double tSphereSpacing = 5.1;
    const common::Coordinate tBBoxMin{0.0, 0.0, 0.0};
    const common::Coordinate tBBoxMax{10.0, 10.0, 10.0};
    const SpherePatternData tData{tBBoxMin, tBBoxMax, tSphereRadius, tSphereSpacing};
    const common::Coordinate tNumSpheres{calculate_num_spheres_in_each_direction(tData)};
    EXPECT_EQ(tNumSpheres.x, 3);
    EXPECT_EQ(tNumSpheres.y, 3);
    EXPECT_EQ(tNumSpheres.z, 3);
    const common::Coordinate tStart{calculate_sphere_pattern_start(tNumSpheres, tData)};
    const Coordinate tGold{-0.1, -0.1, -0.1};
    constexpr double tTol{1e-10};
    ASSERT_NEAR(tStart.x, tGold.x, tTol);
    ASSERT_NEAR(tStart.y, tGold.y, tTol);
    ASSERT_NEAR(tStart.z, tGold.z, tTol);
}

TEST_F(PlatoTestKrino, SphereBuilderCalculateNumSpheresAndStart_SpheresNotIntersectingWithBoundary)
{
    constexpr double tSphereRadius = 0.1;
    constexpr double tSphereSpacing = 2.0;
    const common::Coordinate tBBoxMin{0.0, 0.0, 0.0};
    const common::Coordinate tBBoxMax{10.0, 10.0, 10.0};
    const SpherePatternData tData{tBBoxMin, tBBoxMax, tSphereRadius, tSphereSpacing};
    const common::Coordinate tNumSpheres{calculate_num_spheres_in_each_direction(tData)};
    EXPECT_EQ(tNumSpheres.x, 5);
    EXPECT_EQ(tNumSpheres.y, 5);
    EXPECT_EQ(tNumSpheres.z, 5);
    const common::Coordinate tStart{calculate_sphere_pattern_start(tNumSpheres, tData)};
    const Coordinate tGold{1, 1, 1};
    constexpr double tTol{1e-10};
    ASSERT_NEAR(tStart.x, tGold.x, tTol);
    ASSERT_NEAR(tStart.y, tGold.y, tTol);
    ASSERT_NEAR(tStart.z, tGold.z, tTol);
}

TEST_F(PlatoTestKrino, SphereBuilderCalculateNumSpheresAndStart_OneSphere)
{
    constexpr double tSphereRadius = 0.1;
    constexpr double tSphereSpacing = 100.0;
    const common::Coordinate tBBoxMin{0.0, 0.0, 0.0};
    const common::Coordinate tBBoxMax{10.0, 10.0, 10.0};
    const SpherePatternData tData{tBBoxMin, tBBoxMax, tSphereRadius, tSphereSpacing};
    const common::Coordinate tNumSpheres{calculate_num_spheres_in_each_direction(tData)};
    EXPECT_EQ(tNumSpheres.x, 1);
    EXPECT_EQ(tNumSpheres.y, 1);
    EXPECT_EQ(tNumSpheres.z, 1);
    const common::Coordinate tStart{calculate_sphere_pattern_start(tNumSpheres, tData)};
    const Coordinate tGold{5, 5, 5};
    constexpr double tTol{1e-10};
    ASSERT_NEAR(tStart.x, tGold.x, tTol);
    ASSERT_NEAR(tStart.y, tGold.y, tTol);
    ASSERT_NEAR(tStart.z, tGold.z, tTol);
}

TEST_F(PlatoTestKrino, SphereBuilderGenerateSpheres)
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
