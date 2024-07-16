#include "plato/third_party_integration/stk_search/Utilities.hpp"

#include <memory>
#include <numeric>

namespace plato::third_party_integration::stk_search
{
namespace
{
const auto kDefaultSTKSearch = stk::search::KDTREE;

}  // namespace

SearchPoint convert_coordinate(const common::Coordinate& aCoordinate)
{
    return {aCoordinate.x, aCoordinate.y, aCoordinate.z};
}

common::Coordinate convert_search_point(const SearchPoint& aSearchPoint)
{
    return {aSearchPoint.get_x_min(), aSearchPoint.get_y_min(), aSearchPoint.get_z_min()};
}

SearchResults find_points_in_sphere(const common::Coordinate aCenter,
                                    const double aSearchRadius,
                                    const std::vector<SearchPointWithIdentifier>& aLocalSearchPointWithIdentifiers,
                                    const boost::mpi::communicator& aCommunicator)
{
    const SearchSphere tSearchSphere = create_sphere(aCenter, STKRadius{aSearchRadius});
    const Identifier tSphereIdentifier{0, aCommunicator.rank()};
    const std::vector<SearchSphereWithIdentifier> tSearchDomain = {{tSearchSphere, tSphereIdentifier}};

    SearchResults tSearchResults;
    constexpr bool tEnforceSearchSymmetry = false;
    stk::search::coarse_search(tSearchDomain, aLocalSearchPointWithIdentifiers, kDefaultSTKSearch, aCommunicator,
                               tSearchResults, tEnforceSearchSymmetry);
    return tSearchResults;
}

SearchSphere create_sphere(const common::Coordinate& aCenter, const STKRadius aRadius)
{
    return SearchSphere{{aCenter.x, aCenter.y, aCenter.z}, aRadius.mValue};
}

std::vector<SearchPointWithIdentifier> search_points_with_identifiers(
    const std::vector<common::Coordinate>& aCoordinates)
{
    std::vector<SearchPointWithIdentifier> tSearchPointWithIdentifiers;
    int tIndex = -1;
    std::transform(
        aCoordinates.begin(), aCoordinates.end(), std::back_inserter(tSearchPointWithIdentifiers),
        [tIndex](const auto& iNode) mutable {
            return SearchPointWithIdentifier{SearchPoint{iNode.x, iNode.y, iNode.z}, Identifier{++tIndex, 0}};
        });
    return tSearchPointWithIdentifiers;
}

bool is_in_search_results(const Identifier aIdentifier, const SearchResults& aSearchResults)
{
    return std::any_of(aSearchResults.begin(), aSearchResults.end(),
                       [aIdentifier](auto tSearchResult) { return tSearchResult.second == aIdentifier; });
}

}  // namespace plato::third_party_integration::stk_search
