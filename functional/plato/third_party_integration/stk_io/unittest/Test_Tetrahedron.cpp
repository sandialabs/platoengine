#include <gtest/gtest.h>

#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/Tetrahedron.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/SensitivityTestUtilities.hpp"

namespace plato::third_party_integration::stk_io::unittest
{
namespace
{
Tetrahedron create_unit_tetrahedron()
{
    const common::Coordinate tPointA{.x = 0, .y = 0, .z = 0};
    const common::Coordinate tPointB{.x = 1, .y = 0, .z = 0};
    const common::Coordinate tPointC{.x = 0, .y = 1, .z = 0};
    const common::Coordinate tPointD{.x = 0, .y = 0, .z = 1};

    return Tetrahedron{.p0 = tPointA, .p1 = tPointB, .p2 = tPointC, .p3 = tPointD};
}

}  // namespace

TEST(Tetrahedron, Volume)
{
    const Tetrahedron tTet = create_unit_tetrahedron();
    const double tResult = tTet.volume();
    constexpr double tGold = 1.0 / 6.0;

    EXPECT_DOUBLE_EQ(tResult, tGold);
}

TEST(Tetrahedron, Centroid)
{
    const Tetrahedron tTet = create_unit_tetrahedron();
    const common::Coordinate tResult = tTet.centroid();
    const common::Coordinate tGold{1.0 / 4.0, 1.0 / 4.0, 1.0 / 4.0};

    common::test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Tetrahedron Centroid"));
}

TEST(Tetrahedron, VolumeVertexSensitivities)
{
    constexpr double tBase{88.0};
    constexpr double tWidth{71.0};
    constexpr double tHeight{21.0};

    const auto tAxesAlignedTet = Tetrahedron{.p0 = common::Coordinate{.x = 0, .y = 0, .z = 0},
                                             .p1 = common::Coordinate{.x = tBase, .y = 0, .z = 0},
                                             .p2 = common::Coordinate{.x = 0, .y = tWidth, .z = 0},
                                             .p3 = common::Coordinate{.x = 0, .y = 0, .z = tHeight}};

    const auto tVertexSensitivities = tAxesAlignedTet.volumeVertexSensitivities();

    const auto tExpectedSensitivities = test_utilities::axes_aligned_tet_volume_sensitivities(
        test_utilities::Cuboid{.mLength = tBase, .mWidth = tWidth, .mHeight = tHeight});
    test_utilities::test_nodal_sensitivities_with_expected(tVertexSensitivities, tExpectedSensitivities,
                                                           TEST_CONTEXT("Tetrahedron volume vertex sensitivities"));
}

}  // namespace plato::third_party_integration::stk_io::unittest
