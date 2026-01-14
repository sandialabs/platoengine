#include "plato/third_party_integration/common/BoundingBox.hpp"

namespace plato::third_party_integration::common
{

auto bounding_box(const BoundingBox& aBoundingBoxOne, const BoundingBox& aBoundingBoxTwo) -> BoundingBox
{
    const auto tMinimum = common::minimum_entries(aBoundingBoxOne.first, aBoundingBoxTwo.first);
    const auto tMaximum = common::maximum_entries(aBoundingBoxOne.second, aBoundingBoxTwo.second);
    return std::make_pair(tMinimum, tMaximum);
}

}  // namespace plato::third_party_integration::common
