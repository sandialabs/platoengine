#include <gtest/gtest.h>

#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/Hexahedron.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/SensitivityTestUtilities.hpp"

namespace plato::third_party_integration::stk_io::unittest
{
namespace
{
[[nodiscard]] Hexahedron create_coordinate_axis_aligned_hexahedron_from_cuboid(const test_utilities::Cuboid& aCuboid)
{
    const common::Coordinate tPoint0{.x = 0, .y = 0, .z = 0};
    const common::Coordinate tPoint1{.x = aCuboid.mLength, .y = 0, .z = 0};
    const common::Coordinate tPoint2{.x = aCuboid.mLength, .y = aCuboid.mWidth, .z = 0};
    const common::Coordinate tPoint3{.x = 0, .y = aCuboid.mWidth, .z = 0};
    const common::Coordinate tPoint4{.x = 0, .y = 0, .z = aCuboid.mHeight};
    const common::Coordinate tPoint5{.x = aCuboid.mLength, .y = 0, .z = aCuboid.mHeight};
    const common::Coordinate tPoint6{.x = aCuboid.mLength, .y = aCuboid.mWidth, .z = aCuboid.mHeight};
    const common::Coordinate tPoint7{.x = 0, .y = aCuboid.mWidth, .z = aCuboid.mHeight};

    return Hexahedron{.mP000 = tPoint0,
                      .mP100 = tPoint1,
                      .mP110 = tPoint2,
                      .mP010 = tPoint3,
                      .mP001 = tPoint4,
                      .mP101 = tPoint5,
                      .mP111 = tPoint6,
                      .mP011 = tPoint7};
}
}  // namespace

TEST(Hexahedron, Volume)
{
    constexpr test_utilities::Cuboid tDefiningCuboid{.mLength = 2.0, .mWidth = 4.0, .mHeight = 8.0};
    const Hexahedron tHex = create_coordinate_axis_aligned_hexahedron_from_cuboid(tDefiningCuboid);
    const double tResult = tHex.volume();
    constexpr double tGold = tDefiningCuboid.mLength * tDefiningCuboid.mWidth * tDefiningCuboid.mHeight;

    EXPECT_DOUBLE_EQ(tResult, tGold);
}

TEST(Hexahedron, VolumeOfPerturbedUnitHex)
{
    Hexahedron tHex = create_coordinate_axis_aligned_hexahedron_from_cuboid(
        test_utilities::Cuboid{.mLength = 1.0, .mWidth = 1.0, .mHeight = 1.0});

    tHex.mP100.x += 0.1;
    const double tResult = tHex.volume();
    constexpr double tGold = 41. / 40;  // computed via matlab

    EXPECT_DOUBLE_EQ(tResult, tGold);
}

TEST(Hexahedron, Centroid)
{
    constexpr test_utilities::Cuboid tDefiningCuboid{.mLength = 86.0, .mWidth = 21.0, .mHeight = 38.0};
    const Hexahedron tHex = create_coordinate_axis_aligned_hexahedron_from_cuboid(tDefiningCuboid);
    const auto tResult = tHex.centroid();
    const common::Coordinate tGold{tDefiningCuboid.mLength / 2, tDefiningCuboid.mWidth / 2,
                                   tDefiningCuboid.mHeight / 2};

    common::test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Hexahedron Centroid"));
}

TEST(Hexahedron, CentroidOfParallelepiped)
{
    const common::Coordinate tCornerPoint{.x = 0.0, .y = 0.0, .z = 0.0};
    const common::Coordinate tVector1{.x = 21.0, .y = 12.0, .z = 91.0};
    const common::Coordinate tVector2{.x = 39.0, .y = 71.0, .z = 12.0};
    const common::Coordinate tVector3{.x = 88.0, .y = 35.0, .z = 84.0};
    const Hexahedron tHex{.mP000 = tCornerPoint,
                          .mP100 = tCornerPoint + tVector1,
                          .mP110 = tCornerPoint + tVector1 + tVector2,
                          .mP010 = tCornerPoint + tVector2,
                          .mP001 = tCornerPoint + tVector3,
                          .mP101 = tCornerPoint + tVector1 + tVector3,
                          .mP111 = tCornerPoint + tVector1 + tVector2 + tVector3,
                          .mP011 = tCornerPoint + tVector2 + tVector3};
    const auto tResult = tHex.centroid();
    const common::Coordinate tGold{
        (tHex.mP000 + tHex.mP100 + tHex.mP110 + tHex.mP010 + tHex.mP001 + tHex.mP101 + tHex.mP111 + tHex.mP011) / 8};

    common::test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Parallelepiped Centroid"));
}

TEST(Hexahedron, VolumeVertexSensitivities)
{
    constexpr test_utilities::Cuboid tDefiningCuboid{.mLength = 88.0, .mWidth = 71.0, .mHeight = 59.0};
    const Hexahedron tHex = create_coordinate_axis_aligned_hexahedron_from_cuboid(tDefiningCuboid);

    const auto tVertexSensitivities = tHex.volumeVertexSensitivities();

    const auto tExpectedSensitivities = test_utilities::axes_aligned_hex_volume_sensitivities(tDefiningCuboid);
    test_utilities::test_nodal_sensitivities_with_expected(tVertexSensitivities, tExpectedSensitivities,
                                                           TEST_CONTEXT("Hexahedron volume vertex sensitivities"));
}
}  // namespace plato::third_party_integration::stk_io::unittest
