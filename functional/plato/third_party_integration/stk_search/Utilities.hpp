#ifndef PLATO_THIRDPARTYINTEGRATION_STKSEARCH_UTILITIES
#define PLATO_THIRDPARTYINTEGRATION_STKSEARCH_UTILITIES

#include <boost/mpi/communicator.hpp>
#include <stk_search/Box.hpp>
#include <stk_search/CoarseSearch.hpp>
#include <stk_search/Point.hpp>
#include <vector>

#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::third_party_integration::stk_search
{

using Identifier = stk::search::IdentProc<int, int>;
using SearchSphere = stk::search::Sphere<double>;
using SearchSphereWithIdentifier = std::pair<SearchSphere, Identifier>;
using SearchBox = stk::search::Box<double>;
using SearchBoxWithIdentifier = std::pair<SearchBox, Identifier>;

using SearchPoint = stk::search::Point<double>;
using SearchPointWithIdentifier = std::pair<SearchPoint, Identifier>;
using SearchResults = std::vector<std::pair<Identifier, Identifier>>;

using STKRadius = utilities::NamedType<double, struct STKRadiusTag>;
using STKBoxCenter = utilities::NamedType<common::Coordinate, struct STKBoxCenterTag>;
using STKBoxDimension = utilities::NamedType<common::Coordinate, struct STKBoxDimensionTag>;

template <typename T>
concept STKSearchGeometry = std::is_same_v<T, SearchBoxWithIdentifier> || std::is_same_v<T, SearchSphereWithIdentifier>;

/// @brief Convert the plato functional coordinate object @a aCoordinate to a STK search point for use in stk searches
[[nodiscard]] SearchPoint convert_coordinate(const common::Coordinate& aCoordinate);

/// @brief Convert the stk search point @a aSearchPoint into a plato functional coordinate object
[[nodiscard]] common::Coordinate convert_search_point(const SearchPoint& aSearchPoint);

/// @brief Helper function that calls the STK search procedures on a vector of SearchGeometry with IDs @a
/// aLocalSearchGeometryWithIdentifier using the vector of search points with identifiers @a
/// aLocalSearchPointsWithIdentifiers
template <STKSearchGeometry SearchGeometry>
[[nodiscard]] SearchResults stk_search(const std::vector<SearchGeometry>& aLocalSearchGeometryWithIdentifier,
                                       const std::vector<SearchPointWithIdentifier>& aLocalSearchPointWithIdentifiers,
                                       const boost::mpi::communicator& aCommunicator);

/// @brief Take a a center coordinate @a aCenter, and a strongly typed double @a aRadius to create a stk search sphere
/// object
[[nodiscard]] SearchSphere create_sphere(const common::Coordinate& aCenter, const STKRadius aRadius);

/// @brief Take a a center coordinate @a aCenter, dimensions @a aDimensions to create a stk search box
[[nodiscard]] SearchBox search_box(const STKBoxCenter& aCenter, const STKBoxDimension& aDimensions);

/// @brief Take a vector of coordinates @a aCoordinates and assign them simple identifiers assuming no additional ranks,
/// return a STK searchable object
[[nodiscard]] std::vector<SearchPointWithIdentifier> search_points_with_identifiers(
    const std::vector<common::Coordinate>& aCoordinates);

/// @brief Determine whether a specific identifier @a aIdentifier is within the search results @a aSearchResults
[[nodiscard]] bool is_in_search_results(const Identifier aIdentifier, const SearchResults& aSearchResults);

template <STKSearchGeometry SearchGeometry>
SearchResults stk_search(const std::vector<SearchGeometry>& aLocalSearchGeometryWithIdentifier,
                         const std::vector<SearchPointWithIdentifier>& aLocalSearchPointWithIdentifiers,
                         const boost::mpi::communicator& aCommunicator)
{
    const auto tDefaultSTKSearch = stk::search::KDTREE;
    constexpr bool tEnforceSearchSymmetry = false;
    constexpr bool tAutoSwapDomainAndRange = false;

    if (aLocalSearchGeometryWithIdentifier.empty() || aLocalSearchPointWithIdentifiers.empty())
    {
        return SearchResults{};
    }

    SearchResults tSearchResults;
    stk::search::coarse_search(aLocalSearchGeometryWithIdentifier, aLocalSearchPointWithIdentifiers, tDefaultSTKSearch,
                               aCommunicator, tSearchResults, tEnforceSearchSymmetry, tAutoSwapDomainAndRange);
    return tSearchResults;
}

}  // namespace plato::third_party_integration::stk_search

#endif
