#include <gtest/gtest.h>

#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/krino/SensitivityTriangle.hpp"

namespace plato::third_party_integration::krino::unittest
{

using namespace plato::third_party_integration::common;

namespace
{
std::array<Vector3, 3> kDummyNormalSensitivity{Vector3{0., 0., 0.}, Vector3{0., 0., 0.}, Vector3{0., 0., 0.}};
Vector3 kDummyAreaSensitivity{0., 0., 0.};
SensitivityNode kNode1{.mGlobalNodeID = 23,
                       .mCoordinates = {1., 0., 0.},
                       .mAreaSensitivity = kDummyAreaSensitivity,
                       .mNormalSensitivity = kDummyNormalSensitivity};
SensitivityNode kNode2{.mGlobalNodeID = 55,
                       .mCoordinates = {0., 1., 0.},
                       .mAreaSensitivity = kDummyAreaSensitivity,
                       .mNormalSensitivity = kDummyNormalSensitivity};
SensitivityNode kNode3{.mGlobalNodeID = 2,
                       .mCoordinates = {1., 0., 1.},
                       .mAreaSensitivity = kDummyAreaSensitivity,
                       .mNormalSensitivity = kDummyNormalSensitivity};
SensitivityTriangle kTriangle{kNode1, kNode2, kNode3};
}  // namespace

TEST(SensitivityTriangle, Normal)
{
    const UnitVector3 tNormal = kTriangle.normal();
    const common::UnitVector3 tGold{1.0, 1.0, 0.0};
    common::test_utilities::test_double_equality_of_components(tNormal, tGold,
                                                               TEST_CONTEXT("SensitivityTriangle normal"));
}

TEST(SensitivityTriangle, Area)
{
    const double tArea = kTriangle.area();
    const double tGold = std::sqrt(2.0) / 2.0;
    EXPECT_EQ(tArea, tGold) << TEST_CONTEXT("SensitivityTriangle area");
}

}  // namespace plato::third_party_integration::krino::unittest
