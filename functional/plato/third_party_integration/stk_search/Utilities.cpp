#include "plato/third_party_integration/stk_search/Utilities.hpp"

#include <iterator>
#include <stk_search/SearchMethod.hpp>

namespace plato::third_party_integration::stk_search
{

SearchPoint convert_coordinate(const common::Coordinate& aCoordinate)
{
    return {aCoordinate.x, aCoordinate.y, aCoordinate.z};
}

common::Coordinate convert_search_point(const SearchPoint& aSearchPoint)
{
    return {aSearchPoint.get_x_min(), aSearchPoint.get_y_min(), aSearchPoint.get_z_min()};
}

SearchSphere create_sphere(const common::Coordinate& aCenter, const STKRadius aRadius)
{
    return SearchSphere{{aCenter.x, aCenter.y, aCenter.z}, aRadius.mValue};
}

SearchBox search_box(const STKBoxCenter& aCenter, const STKBoxDimension& aDimensions)
{
    return SearchBox{aCenter.mValue.x - aDimensions.mValue.x / 2.0, aCenter.mValue.y - aDimensions.mValue.y / 2.0,
                     aCenter.mValue.z - aDimensions.mValue.z / 2.0, aCenter.mValue.x + aDimensions.mValue.x / 2.0,
                     aCenter.mValue.y + aDimensions.mValue.y / 2.0, aCenter.mValue.z + aDimensions.mValue.z / 2.0};
}

std::vector<SearchPointWithIdentifier> search_points_with_identifiers(
    const std::vector<common::Coordinate>& aCoordinates)
{
    std::vector<SearchPointWithIdentifier> tSearchPointWithIdentifiers;
    int tIndex = -1;
    std::transform(
        aCoordinates.begin(), aCoordinates.end(), std::back_inserter(tSearchPointWithIdentifiers),
        [tIndex](const auto& iNode) mutable
        { return SearchPointWithIdentifier{SearchPoint{iNode.x, iNode.y, iNode.z}, Identifier{++tIndex, 0}}; });
    return tSearchPointWithIdentifiers;
}

bool is_in_search_results(const Identifier aIdentifier, const SearchResults& aSearchResults)
{
    return std::any_of(aSearchResults.begin(), aSearchResults.end(),
                       [aIdentifier](auto tSearchResult) { return tSearchResult.second == aIdentifier; });
}

}  // namespace plato::third_party_integration::stk_search
