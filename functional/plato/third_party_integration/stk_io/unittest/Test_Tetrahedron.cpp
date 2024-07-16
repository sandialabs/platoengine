#include <gtest/gtest.h>

#include <filesystem>
#include <iomanip>

#include "plato/third_party_integration/common/unittest/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/Tetrahedron.hpp"

namespace plato::third_party_integration::stk_io::unittest
{
namespace
{
Tetrahedron create_unit_tetrahedron()
{
    const common::Coordinate tPointA{0, 0, 0};
    const common::Coordinate tPointB{1, 0, 0};
    const common::Coordinate tPointC{0, 1, 0};
    const common::Coordinate tPointD{0, 0, 1};

    return Tetrahedron{tPointA, tPointB, tPointC, tPointD};
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

    common::unittest::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Tetrahedron Centroid"));
}

}  // namespace plato::third_party_integration::stk_io::unittest
