#include "plato/third_party_integration/stk_search/Utilities.hpp"

#include <algorithm>
#include <iterator>
#include <vector>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::stk_search
{

auto convert_coordinate(const common::Coordinate& aCoordinate) -> SearchPoint
{
    return {aCoordinate.mX, aCoordinate.mY, aCoordinate.mZ};
}

auto convert_search_point(const SearchPoint& aSearchPoint) -> common::Coordinate
{
    return {aSearchPoint.get_x_min(), aSearchPoint.get_y_min(), aSearchPoint.get_z_min()};
}

auto create_sphere(const common::Coordinate& aCenter, const STKRadius aRadius) -> SearchSphere
{
    return SearchSphere{{aCenter.mX, aCenter.mY, aCenter.mZ}, aRadius.mValue};
}

auto search_box(const STKBoxCenter& aCenter, const STKBoxDimension& aDimensions) -> SearchBox
{
    const auto tXExtent = aDimensions.mValue.mX / 2.0;
    const auto tYExtent = aDimensions.mValue.mY / 2.0;
    const auto tZExtent = aDimensions.mValue.mZ / 2.0;
    return SearchBox{aCenter.mValue.mX - tXExtent, aCenter.mValue.mY - tYExtent, aCenter.mValue.mZ - tZExtent,
                     aCenter.mValue.mX + tXExtent, aCenter.mValue.mY + tYExtent, aCenter.mValue.mZ + tZExtent};
}

auto search_points_with_identifiers(const std::vector<common::Coordinate>& aCoordinates)
    -> std::vector<SearchPointWithIdentifier>
{
    std::vector<SearchPointWithIdentifier> tSearchPointWithIdentifiers;
    std::transform(
        aCoordinates.begin(), aCoordinates.end(), std::back_inserter(tSearchPointWithIdentifiers),
        [tIndex = -1](const auto& aNode) mutable
        { return SearchPointWithIdentifier{SearchPoint{aNode.mX, aNode.mY, aNode.mZ}, Identifier{++tIndex, 0}}; });
    return tSearchPointWithIdentifiers;
}

auto is_in_search_results(const Identifier aIdentifier, const SearchResults& aSearchResults) -> bool
{
    return std::any_of(aSearchResults.begin(), aSearchResults.end(),
                       [aIdentifier](auto aSearchResult) { return aSearchResult.second == aIdentifier; });
}

}  // namespace plato::third_party_integration::stk_search
