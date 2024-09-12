#include <gtest/gtest.h>

#include "plato/third_party_integration/common/Vector3Hash.hpp"

namespace plato::third_party_integration::common::unittest
{
TEST(Coordinate, HashX)
{
    const auto tCoordinate1 = Coordinate{0.0, 1.0, 2.0};
    const auto tCoordinate2 = Coordinate{1.0, 1.0, 2.0};
    EXPECT_NE(hash_value(tCoordinate1), hash_value(tCoordinate2));
}

TEST(Coordinate, HashY)
{
    const auto tCoordinate1 = Coordinate{0.0, 1.0, 2.0};
    const auto tCoordinate2 = Coordinate{0.0, 2.0, 2.0};
    EXPECT_NE(hash_value(tCoordinate1), hash_value(tCoordinate2));
}

TEST(Coordinate, HashZ)
{
    const auto tCoordinate1 = Coordinate{0.0, 1.0, 2.0};
    const auto tCoordinate2 = Coordinate{0.0, 1.0, -2.0};
    EXPECT_NE(hash_value(tCoordinate1), hash_value(tCoordinate2));
}

TEST(Coordinate, HashAll)
{
    const auto tCoordinate1 = Coordinate{0.0, 1.0, 2.0};
    const auto tCoordinate2 = Coordinate{4.0, -1.0, 6.0};
    EXPECT_NE(hash_value(tCoordinate1), hash_value(tCoordinate2));
}

TEST(Coordinate, HashNext)
{
    const auto tCoordinate1 = Coordinate{0.0, 1.0, 2.0};
    const auto tCoordinate2 = Coordinate{std::nextafter(0.0, 1.0), 1.0, 2.0};
    EXPECT_NE(hash_value(tCoordinate1), hash_value(tCoordinate2));
}

TEST(Coordinate, HashSame)
{
    constexpr auto tNonExactValue = 9.057919370756192156e-01;
    const auto tCoordinate1 = Coordinate{tNonExactValue, 1.0, 2.0};
    const auto tCoordinate2 = tCoordinate1;
    EXPECT_EQ(hash_value(tCoordinate1), hash_value(tCoordinate2));
}

}  // namespace plato::third_party_integration::common::unittest