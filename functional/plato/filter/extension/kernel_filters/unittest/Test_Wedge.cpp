#include <gtest/gtest.h>

#include <numbers>

#include "plato/filter/extension/kernel_filters/Wedge.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"

namespace plato::filter::extension::kernel_filters::unittest
{

namespace
{
const auto kPositiveXPlane = Plane({0, 0, 0}, {1, 0, 0});
constexpr auto kSqrtTwoOverTwo = std::numbers::sqrt2 / 2.0;
const auto kFortyFive = Plane({0., 0., 0}, {kSqrtTwoOverTwo, kSqrtTwoOverTwo, 0});

}  // namespace

TEST(Plane, PositiveQuadrantWedge)
{
    namespace tpic = third_party_integration::common;
    namespace tpict = third_party_integration::common::test_utilities;
    constexpr auto tTolerance = 1e-15;
    const auto tWedge = positive_quadrant_wedge(45.0);
    tpict::test_near_equality_of_components(static_cast<tpic::Vector3>(tWedge.mFirstPlane.mUnitNormal),
                                            static_cast<tpic::Vector3>(kPositiveXPlane.mUnitNormal), tTolerance,
                                            TEST_CONTEXT("First plane normal."));
    tpict::test_near_equality_of_components(tWedge.mFirstPlane.mCenter, kPositiveXPlane.mCenter, tTolerance,
                                            TEST_CONTEXT("First plane center."));
    tpict::test_near_equality_of_components(static_cast<tpic::Vector3>(tWedge.mSecondPlane.mUnitNormal),
                                            static_cast<tpic::Vector3>(kFortyFive.mUnitNormal), tTolerance,
                                            TEST_CONTEXT("Second plane normal."));
    tpict::test_near_equality_of_components(tWedge.mSecondPlane.mCenter, kFortyFive.mCenter, tTolerance,
                                            TEST_CONTEXT("Second plane center."));
    EXPECT_EQ(tWedge.mRepeats, 7U);
}

}  // namespace plato::filter::extension::kernel_filters::unittest
