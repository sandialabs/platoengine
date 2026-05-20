#include <gtest/gtest.h>

#include <cmath>

#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/Triangle.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/SensitivityTestUtilities.hpp"

namespace plato::third_party_integration::stk_io::unittest
{
namespace
{
Triangle create_unit_triangle()
{
    const common::Coordinate tPointA{.mX = 1, .mY = 0, .mZ = 0};
    const common::Coordinate tPointB{.mX = 0, .mY = 1, .mZ = 0};
    const common::Coordinate tPointC{.mX = 0, .mY = 0, .mZ = 1};

    return Triangle{.mP0 = tPointA, .mP1 = tPointB, .mP2 = tPointC};
}
}  // namespace

TEST(Triangle, Volume)
{
    const Triangle tTri = create_unit_triangle();
    const double tResult = tTri.volume();

    const common::Vector3 tVectorA{tTri.mP1 - tTri.mP0};
    const common::Vector3 tVectorB{tTri.mP2 - tTri.mP0};
    const common::Vector3 tCrossBA = cross(tVectorB, tVectorA);
    const double tGold = std::sqrt(dot(tCrossBA, tCrossBA)) * 0.5;

    EXPECT_DOUBLE_EQ(tResult, tGold);
}

TEST(Triangle, Centroid)
{
    const Triangle tTri = create_unit_triangle();
    const common::Coordinate tResult = tTri.centroid();

    const common::Coordinate tGold{.mX = 1.0 / 3.0, .mY = 1.0 / 3.0, .mZ = 1.0 / 3.0};
    common::test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Triangle centroid"));
}

TEST(Triangle, VolumeVertexSensitivities)
{
    constexpr double tBase{4.0};
    constexpr double tHeight{24.0};
    const auto tPlanarTri = Triangle{.mP0 = common::Coordinate{.mX = 0, .mY = 0, .mZ = 0},
                                     .mP1 = common::Coordinate{.mX = tBase, .mY = 0, .mZ = 0},
                                     .mP2 = common::Coordinate{.mX = 0, .mY = tHeight, .mZ = 0}};

    const auto tVertexSensitivities = tPlanarTri.volumeVertexSensitivities();
    const auto tExpectedSensitivities = test_utilities::axes_aligned_tri_volume_sensitivities(
        test_utilities::Rectangle{.mLength = tBase, .mWidth = tHeight});

    test_utilities::test_nodal_sensitivities_with_expected(tVertexSensitivities, tExpectedSensitivities,
                                                           TEST_CONTEXT("Triangle volume vertex sensitivities"));
}

}  // namespace plato::third_party_integration::stk_io::unittest
