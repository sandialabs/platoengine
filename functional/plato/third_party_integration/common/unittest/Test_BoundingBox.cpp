#include <gtest/gtest.h>

#include "plato/third_party_integration/common/BoundingBox.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::common::unittest
{

TEST(BoundingBox, BoundingBoxFromCoordinates)
{
    const auto tCoordinates = std::vector<Coordinate>{{5, 5, 5}, {-1, 0, 1}, {2, 2, 2}, {0, 0, 0}};
    const auto tGold = BoundingBox{{-1, 0, 0}, {5, 5, 5}};
    const auto tResult = bounding_box(tCoordinates);
    EXPECT_EQ(tResult, tGold);
}

TEST(BoundingBox, BoundingBoxFromBoundingBoxes)
{
    const auto tBoundingBoxOne = BoundingBox{{-1, -2, 0}, {1, 0, 1}};
    const auto tBoundingBoxTwo = BoundingBox{{0, -1, -3}, {1, 2, 3}};
    const auto tGold = BoundingBox{{-1, -2, -3}, {1, 2, 3}};
    const auto tResult = bounding_box(tBoundingBoxOne, tBoundingBoxTwo);
    EXPECT_EQ(tResult, tGold);
}

}  // namespace plato::third_party_integration::common::unittest
