#ifndef PLATO_THIRDPARTYINTEGRATION_COMMON_BOUNDINGBOX
#define PLATO_THIRDPARTYINTEGRATION_COMMON_BOUNDINGBOX

#include <numeric>
#include <ranges>
#include <utility>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::common
{

using BoundingBox = std::pair<Coordinate, Coordinate>;

///@brief Take a vector of coordinates @a aCoordinates and compute its bounding box
[[nodiscard]] auto bounding_box(const std::ranges::forward_range auto& aCoordinates) -> BoundingBox;

///@brief Take two bounding boxes @a aBoundingBoxOne and @a aBoundingBoxTwo and return their bounding box
[[nodiscard]] auto bounding_box(const BoundingBox& aBoundingBoxOne, const BoundingBox& aBoundingBoxTwo) -> BoundingBox;

auto bounding_box(const std::ranges::forward_range auto& aCoordinates) -> BoundingBox
{
    return std::accumulate(aCoordinates.begin(), aCoordinates.end(),
                           BoundingBox{aCoordinates.front(), aCoordinates.front()},
                           [](const auto& aBoundingBox, const auto& aCoordinate)
                           {
                               return BoundingBox{common::minimum_entries(aCoordinate, aBoundingBox.first),
                                                  common::maximum_entries(aCoordinate, aBoundingBox.second)};
                           });
}

}  // namespace plato::third_party_integration::common

#endif
