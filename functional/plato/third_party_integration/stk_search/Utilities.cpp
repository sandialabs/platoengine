#include "plato/third_party_integration/stk_search/Utilities.hpp"

#include <algorithm>
#include <iterator>
#include <vector>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::stk_search
{

auto convert_coordinate(const common::Coordinate& aCoordinate) -> SearchPoint
{
    return {aCoordinate.x, aCoordinate.y, aCoordinate.z};
}

auto convert_search_point(const SearchPoint& aSearchPoint) -> common::Coordinate
{
    return {aSearchPoint.get_x_min(), aSearchPoint.get_y_min(), aSearchPoint.get_z_min()};
}

auto create_sphere(const common::Coordinate& aCenter, const STKRadius aRadius) -> SearchSphere
{
    return SearchSphere{{aCenter.x, aCenter.y, aCenter.z}, aRadius.mValue};
}

auto search_box(const STKBoxCenter& aCenter, const STKBoxDimension& aDimensions) -> SearchBox
{
    const auto tXExtent = aDimensions.mValue.x / 2.0;
    const auto tYExtent = aDimensions.mValue.y / 2.0;
    const auto tZExtent = aDimensions.mValue.z / 2.0;
    return SearchBox{aCenter.mValue.x - tXExtent, aCenter.mValue.y - tYExtent, aCenter.mValue.z - tZExtent,
                     aCenter.mValue.x + tXExtent, aCenter.mValue.y + tYExtent, aCenter.mValue.z + tZExtent};
}

auto search_points_with_identifiers(const std::vector<common::Coordinate>& aCoordinates)
    -> std::vector<SearchPointWithIdentifier>
{
    std::vector<SearchPointWithIdentifier> tSearchPointWithIdentifiers;
    int tIndex = -1;
    std::transform(
        aCoordinates.begin(), aCoordinates.end(), std::back_inserter(tSearchPointWithIdentifiers),
        [tIndex](const auto& iNode) mutable
        { return SearchPointWithIdentifier{SearchPoint{iNode.x, iNode.y, iNode.z}, Identifier{++tIndex, 0}}; });
    return tSearchPointWithIdentifiers;
}

auto is_in_search_results(const Identifier aIdentifier, const SearchResults& aSearchResults) -> bool
{
    return std::any_of(aSearchResults.begin(), aSearchResults.end(),
                       [aIdentifier](auto tSearchResult) { return tSearchResult.second == aIdentifier; });
}

}  // namespace plato::third_party_integration::stk_search
