#include <gtest/gtest.h>

#include <numbers>

#include "plato/filter/extension/kernel_filters/Wedge.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::filter::extension::kernel_filters::unittest
{

namespace
{
constexpr auto kFortyFiveDegree = 45.0;
const auto kPositiveYPlane = Plane({0, 0, 0}, {0, 1, 0});
constexpr auto kSqrtTwoOverTwo = std::numbers::sqrt2 / 2.0;
const auto kFortyFiveAnglePlane = Plane({0., 0., 0}, {-kSqrtTwoOverTwo, kSqrtTwoOverTwo, 0});

}  // namespace

TEST(Wedge, PositiveQuadrantWedge)
{
    namespace tpic = third_party_integration::common;
    namespace tpict = third_party_integration::common::test_utilities;
    constexpr auto tTolerance = 1e-15;
    const auto tWedge = positive_quadrant_wedge(kFortyFiveDegree);
    tpict::test_near_equality_of_components(static_cast<tpic::Vector3>(tWedge.mFirstPlane.mUnitNormal),
                                            static_cast<tpic::Vector3>(kPositiveYPlane.mUnitNormal), tTolerance,
                                            TEST_CONTEXT("First plane normal."));
    tpict::test_near_equality_of_components(tWedge.mFirstPlane.mCenter, kPositiveYPlane.mCenter, tTolerance,
                                            TEST_CONTEXT("First plane center."));
    tpict::test_near_equality_of_components(static_cast<tpic::Vector3>(tWedge.mSecondPlane.mUnitNormal),
                                            static_cast<tpic::Vector3>(kFortyFiveAnglePlane.mUnitNormal), tTolerance,
                                            TEST_CONTEXT("Second plane normal."));
    tpict::test_near_equality_of_components(tWedge.mSecondPlane.mCenter, kFortyFiveAnglePlane.mCenter, tTolerance,
                                            TEST_CONTEXT("Second plane center."));
    EXPECT_EQ(tWedge.mRepeats, 7U);
}

TEST(Wedge, WedgeToPlanes)
{
    const auto tWedge = positive_quadrant_wedge(kFortyFiveDegree);
    const auto tPlane = wedge_to_planes(tWedge);
    ASSERT_EQ(tPlane.size(), tWedge.mRepeats);
    const auto tGoldNormals = std::vector<third_party_integration::common::Vector3>{
        {-kSqrtTwoOverTwo, kSqrtTwoOverTwo, 0}, {1, 0, 0},  {kSqrtTwoOverTwo, kSqrtTwoOverTwo, 0},  {0, -1, 0},
        {kSqrtTwoOverTwo, -kSqrtTwoOverTwo, 0}, {-1, 0, 0}, {-kSqrtTwoOverTwo, -kSqrtTwoOverTwo, 0}};
    constexpr auto tTolerance = 5e-15;
    for (const auto& [tPlaneEntry, tGoldNormalVector] : utilities::Zip(tPlane, tGoldNormals))
    {
        const auto tResultVector = static_cast<third_party_integration::common::Vector3>(tPlaneEntry.mUnitNormal);
        third_party_integration::common::test_utilities::test_near_equality_of_components(
            tResultVector, tGoldNormalVector, tTolerance, TEST_CONTEXT("Sixty degree wedge points."));
    }
}

}  // namespace plato::filter::extension::kernel_filters::unittest
