#include <gtest/gtest.h>

#include <cmath>
#include <numbers>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/krino/SensitivityTriangle.hpp"

namespace plato::third_party_integration::krino::unittest
{

using namespace plato::third_party_integration::common;

namespace
{
const auto kTriangle = SensitivityTriangle{NodeIDCoordsPair{23, {1., 0., 0.}}, NodeIDCoordsPair{55, {0., 1., 0.}},
                                           NodeIDCoordsPair{2, {1., 0., 1.}}};
}  // namespace

TEST(SensitivityTriangle, Normal)
{
    const Vector3 tNormal = kTriangle.normal();
    const common::UnitVector3 tGold{1.0, 1.0, 0.0};
    const auto tGoldNormal = static_cast<Vector3>(tGold);
    common::test_utilities::test_double_equality_of_components(tNormal, tGoldNormal,
                                                               TEST_CONTEXT("SensitivityTriangle normal"));
}

TEST(SensitivityTriangle, Area)
{
    const double tArea = kTriangle.area();
    const double tGold = std::numbers::sqrt2 / 2.0;
    EXPECT_EQ(tArea, tGold) << TEST_CONTEXT("SensitivityTriangle area");
}

}  // namespace plato::third_party_integration::krino::unittest
