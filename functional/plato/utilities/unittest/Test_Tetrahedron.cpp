#include <gtest/gtest.h>

#include <filesystem>
#include <iomanip>

#include "plato/utilities/Tetrahedron.hpp"

namespace plato::utilities::unittest
{
TEST(Tetrahedron, Volume)
{
    Coordinate tPointA{0, 0, 0};
    Coordinate tPointB{1, 0, 0};
    Coordinate tPointC{0, 1, 0};
    Coordinate tPointD{0, 0, 1};

    const Tetrahedron tTet{tPointA, tPointB, tPointC, tPointD};
    const double tResult = tTet.volume();
    constexpr double tGold = 1.0 / 6.0;

    EXPECT_DOUBLE_EQ(tResult, tGold);
}

}  // namespace plato::utilities::unittest
