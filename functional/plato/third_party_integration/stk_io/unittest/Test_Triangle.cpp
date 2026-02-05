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
    const common::Coordinate tPointA{.x = 1, .y = 0, .z = 0};
    const common::Coordinate tPointB{.x = 0, .y = 1, .z = 0};
    const common::Coordinate tPointC{.x = 0, .y = 0, .z = 1};

    return Triangle{.p0 = tPointA, .p1 = tPointB, .p2 = tPointC};
}
}  // namespace

TEST(Triangle, Volume)
{
    const Triangle tTri = create_unit_triangle();
    const double tResult = tTri.volume();

    const common::Vector3 tVectorA{tTri.p1 - tTri.p0};
    const common::Vector3 tVectorB{tTri.p2 - tTri.p0};
    const common::Vector3 tCrossBA = cross(tVectorB, tVectorA);
    const double tGold = std::sqrt(dot(tCrossBA, tCrossBA)) * 0.5;

    EXPECT_DOUBLE_EQ(tResult, tGold);
}

TEST(Triangle, Centroid)
{
    const Triangle tTri = create_unit_triangle();
    const common::Coordinate tResult = tTri.centroid();

    const common::Coordinate tGold{.x = 1.0 / 3.0, .y = 1.0 / 3.0, .z = 1.0 / 3.0};
    common::test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Triangle centroid"));
}

TEST(Triangle, VolumeVertexSensitivities)
{
    constexpr double tBase{4.0};
    constexpr double tHeight{24.0};
    const auto tPlanarTri = Triangle{.p0 = common::Coordinate{.x = 0, .y = 0, .z = 0},
                                     .p1 = common::Coordinate{.x = tBase, .y = 0, .z = 0},
                                     .p2 = common::Coordinate{.x = 0, .y = tHeight, .z = 0}};

    const auto tVertexSensitivities = tPlanarTri.volumeVertexSensitivities();
    const auto tExpectedSensitivities = test_utilities::axes_aligned_tri_volume_sensitivities(
        test_utilities::Rectangle{.mLength = tBase, .mWidth = tHeight});

    test_utilities::test_nodal_sensitivities_with_expected(tVertexSensitivities, tExpectedSensitivities,
                                                           TEST_CONTEXT("Triangle volume vertex sensitivities"));
}

}  // namespace plato::third_party_integration::stk_io::unittest
