#include "plato/filter/extension/LinearMaskBuilder.hpp"

#include <Teuchos_ArrayViewDecl.hpp>
#include <Teuchos_EReductionType.hpp>
#include <algorithm>
#include <boost/mpi/collectives.hpp>
#include <boost/mpi/collectives/broadcast.hpp>
#include <boost/mpi/communicator.hpp>
#include <cmath>
#include <iterator>
#include <numbers>
#include <utility>

#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshQuantities.hpp"
#include "plato/utilities/IndexRange.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::filter::extension
{

namespace
{
constexpr bool kZeroOut = true;

auto center_coordinates(const mesh::Mesh& aMesh, input_parser::KernelFilterCenteringTypes aCenteringType)
    -> std::vector<third_party_integration::common::Coordinate>
{
    if (aCenteringType == input_parser::KernelFilterCenteringTypes::kElementCentered)
    {
        return mesh::EntityRetrieval{aMesh}.designDomainElementCentroids();
    }
    else
    {
        return mesh::EntityRetrieval{aMesh}.designDomainNodalCoordinates();
    }
}

}  // namespace

LinearMaskBuilder::LinearMaskBuilder(const mesh::Mesh& aMesh,
                                     const input_parser::KernelFilterCenteringTypes aCenteringType,
                                     const SearchRadius aSearchRadius,
                                     const boost::mpi::communicator& aCommunicator)
    : mCommunicator(aCommunicator),
      mSearchRadius(aSearchRadius.mValue),
      mMaximumConnectivityEstimate(detail::maximum_connectivity_estimate(aMesh, aSearchRadius)),
      mGlobalRowCenterCoordinates(center_coordinates(aMesh, aCenteringType)),
      mGlobalNodalCoordinates(mesh::EntityRetrieval{aMesh}.designDomainNodalCoordinates())
{
    generateDistanceMap();
}

LinearMaskBuilder::LinearMaskBuilder(const NodalVector& aNodalCoordinates,
                                     CenterVector aCenters,
                                     const SearchRadius aSearchRadius,
                                     const int aMaximumConnectivityEstimate,
                                     const boost::mpi::communicator& aCommunicator)
    : mCommunicator(aCommunicator),
      mSearchRadius(aSearchRadius.mValue),
      mMaximumConnectivityEstimate(aMaximumConnectivityEstimate),
      mGlobalRowCenterCoordinates(std::move(aCenters.mValue)),
      mGlobalNodalCoordinates(aNodalCoordinates.mValue)
{
    generateDistanceMap();
}

auto LinearMaskBuilder::mask() const -> const third_party_integration::tpetra::TpetraCRSMatrix& { return *mLinearMask; }

double LinearMaskBuilder::unnormalized_weight(const RowSphereGlobalID aSphereID, const ColumnNodeGlobalID aNodeID)
{
    const auto tSphereCoordinate = mGlobalRowCenterCoordinates[aSphereID.mValue];
    const auto tNodalCoordinate = mGlobalNodalCoordinates[aNodeID.mValue];

    const double tDistance = third_party_integration::common::magnitude(tSphereCoordinate - tNodalCoordinate);
    return detail::linear_ramp_weight(Distance{tDistance}, SearchRadius{mSearchRadius});
}

RowMap LinearMaskBuilder::create_normalized_row_map(
    const third_party_integration::stk_search::SearchResults& aSearchResults)
{
    RowMap tRowMap;
    tRowMap.reserve(mGlobalRowCenterCoordinates.size());
    for (const auto& tSearchResult : aSearchResults)
    {
        const auto tSphereID = RowSphereGlobalID{tSearchResult.first.id()};
        const auto tNodeId = ColumnNodeGlobalID{tSearchResult.second.id()};
        const double tWeight = unnormalized_weight(tSphereID, tNodeId);

        detail::add_weight_from_search_result_to_map(tRowMap, tSphereID, tNodeId, Weight{tWeight},
                                                     mMaximumConnectivityEstimate);
    }
    detail::normalize_rows_in_map(tRowMap);
    return tRowMap;
}

void LinearMaskBuilder::create_linear_mask(const RowMap& aRowMap)
{
    namespace tpi = third_party_integration;

    const auto tCommunicator = Teuchos::rcp(new Teuchos::MpiComm<int>(mCommunicator));
    auto tCrsRowMap = Teuchos::rcp(
        new tpi::tpetra::TpetraMap(mGlobalRowCenterCoordinates.size(), tpi::tpetra::kIndexBase, tCommunicator));
    auto tCrsDomainMap = Teuchos::rcp(
        new tpi::tpetra::TpetraMap(mGlobalNodalCoordinates.size(), tpi::tpetra::kIndexBase, tCommunicator));

    mLinearMask = std::make_unique<tpi::tpetra::TpetraCRSMatrix>(tCrsRowMap, mMaximumConnectivityEstimate);

    for (const auto& tRow : aRowMap)
    {
        const auto tRowGlobalID = tRow.first;
        auto tColumnGlobalIDs = aRowMap.at(tRowGlobalID).mNonzeroColumnGlobalIDs;
        auto tColumnWeights = aRowMap.at(tRowGlobalID).mColumnEntryWeights;

        assert(!tColumnGlobalIDs.empty());

        mLinearMask->insertGlobalValues(tRowGlobalID,
                                        Teuchos::ArrayView<tpi::tpetra::TpetraGlobalOrdinal>(tColumnGlobalIDs),
                                        Teuchos::ArrayView<tpi::tpetra::TpetraScalar>(tColumnWeights));
    }
    mLinearMask->fillComplete(tCrsDomainMap, tCrsRowMap);
}

void LinearMaskBuilder::generateDistanceMap()
{
    const auto tSearchResults = detail::distribute_search_vectors_and_stk_search(
        CenterVector{mGlobalRowCenterCoordinates}, NodalVector{mGlobalNodalCoordinates}, mSearchRadius, mCommunicator);

    assert(detail::reduce_search_result_size(tSearchResults, mCommunicator) >= mGlobalRowCenterCoordinates.size());

    const auto tRowMap = create_normalized_row_map(tSearchResults);
    create_linear_mask(tRowMap);
}

namespace detail
{

double linear_ramp_weight(const Distance aDistance, const SearchRadius aSearchRadius)
{
    return std::max(0.0, 1.0 - aDistance.mValue / aSearchRadius.mValue);
}

double filter_volume(const SearchRadius aFilterRadius)
{
    return 4.0 / 3.0 * std::numbers::pi * aFilterRadius.mValue * aFilterRadius.mValue * aFilterRadius.mValue;
}

double filter_area(const SearchRadius aFilterRadius)
{
    return std::numbers::pi * aFilterRadius.mValue * aFilterRadius.mValue;
}

unsigned int maximum_connectivity_estimate(const mesh::Mesh& aMesh, const SearchRadius aFilterRadius)
{
    const double tSmallestElement = mesh::MeshQuantities{aMesh}.smallestDesignDomainElementVolume();
    const double tSearchVolume =
        mesh::EntityCounts{aMesh}.is2D() ? filter_area(aFilterRadius) : filter_volume(aFilterRadius);
    const auto tTotalElements = mesh::EntityCounts{aMesh}.numberOfElements();
    const double tVolume = mesh::MeshQuantities{aMesh}.volume();

    const auto tAverageElementSize = tVolume / tTotalElements;
    const auto tRatioAverageToSmall = tAverageElementSize / tSmallestElement;

    const auto tAverageNodalDensity = mesh::MeshQuantities{aMesh}.averageNodalDensity();

    return static_cast<int>(tAverageNodalDensity * tSearchVolume * tRatioAverageToSmall * kMaxMultiplier);
}

auto create_tpetravector_coordinates(const std::vector<third_party_integration::common::Coordinate>& aCoordinates,
                                     const boost::mpi::communicator& aCommunicator)
    -> third_party_integration::tpetra::TpetraMultiVector
{
    namespace tpi = third_party_integration;

    const auto tCommunicator = Teuchos::rcp(new Teuchos::MpiComm<int>(aCommunicator));
    auto tMap = Teuchos::rcp(new tpi::tpetra::TpetraMap(aCoordinates.size(), tpi::tpetra::kIndexBase, tCommunicator));
    auto tCoordinates = tpi::tpetra::TpetraMultiVector(tMap, tpi::tpetra::kNumberOfCartesianDimensions, kZeroOut);
    tpi::tpetra::distribute_on_tpetra_multivector(aCoordinates, tCoordinates);
    return tCoordinates;
}

auto distribute_search_vectors_and_stk_search(const CenterVector& aCenterVector,
                                              const NodalVector& aNodalVector,
                                              const double aSearchRadius,
                                              const boost::mpi::communicator& aCommunicator)
    -> third_party_integration::stk_search::SearchResults
{
    namespace tpi = third_party_integration;
    tpi::tpetra::TpetraMultiVector tDistributedNodalCoordinates(
        detail::create_tpetravector_coordinates(aNodalVector.mValue, aCommunicator));
    const auto tLocalSearchPointWithIdentifiers =
        detail::stk_search_points(tDistributedNodalCoordinates, aCommunicator.rank());

    const tpi::tpetra::TpetraMultiVector tLocalRowCenterCoordinates =
        detail::create_tpetravector_coordinates(aCenterVector.mValue, aCommunicator);
    const auto tLocalSearchSpheresWithIdentifiers =
        detail::stk_search_spheres(tLocalRowCenterCoordinates, SearchRadius{aSearchRadius}, aCommunicator.rank());

    return tpi::stk_search::perform_stk_search(tLocalSearchSpheresWithIdentifiers, tLocalSearchPointWithIdentifiers,
                                               aCommunicator);
}

auto stk_search_points(const third_party_integration::tpetra::TpetraMultiVector& aNodalCoordinates, const int aRank)
    -> std::vector<third_party_integration::stk_search::SearchPointWithIdentifier>
{
    namespace tpi = third_party_integration;

    const auto tNumberOfLocalElements = tpi::tpetra::number_of_local_elements(aNodalCoordinates.getMap());
    auto tLocalSearchPointWithIdentifiers =
        std::vector<tpi::stk_search::SearchPointWithIdentifier>(tNumberOfLocalElements);

    for (const auto tLocalIndex : utilities::IndexRange{tNumberOfLocalElements})
    {
        const auto tGlobalID = aNodalCoordinates.getMap()->getGlobalElement(tLocalIndex);
        const tpi::stk_search::Identifier tIdentifier{tGlobalID, aRank};
        const tpi::common::Coordinate tCoordinate = tpi::tpetra::multivector_coordinate(aNodalCoordinates, tLocalIndex);
        tLocalSearchPointWithIdentifiers[tLocalIndex] =
            tpi::stk_search::SearchPointWithIdentifier({tpi::stk_search::convert_coordinate(tCoordinate), tIdentifier});
    }
    return tLocalSearchPointWithIdentifiers;
}

auto stk_search_spheres(const third_party_integration::tpetra::TpetraMultiVector& aCenteringCoordinates,
                        const SearchRadius aRadius,
                        const int aRank) -> std::vector<third_party_integration::stk_search::SearchSphereWithIdentifier>
{
    namespace tpi = third_party_integration;

    const auto tNumberOfLocalElements = tpi::tpetra::number_of_local_elements(aCenteringCoordinates.getMap());
    auto tLocalSearchSpheresWithIdentifier =
        std::vector<tpi::stk_search::SearchSphereWithIdentifier>(tNumberOfLocalElements);

    for (const auto tLocalIndex : utilities::IndexRange{tNumberOfLocalElements})
    {
        const auto tGlobalID = aCenteringCoordinates.getMap()->getGlobalElement(tLocalIndex);
        const tpi::stk_search::Identifier tIdentifier{tGlobalID, aRank};
        const tpi::common::Coordinate tCoordinate =
            tpi::tpetra::multivector_coordinate(aCenteringCoordinates, tLocalIndex);
        tLocalSearchSpheresWithIdentifier[tLocalIndex] = tpi::stk_search::SearchSphereWithIdentifier(
            {tpi::stk_search::create_sphere(tCoordinate, tpi::stk_search::STKRadius{aRadius.mValue}), tIdentifier});
    }
    return tLocalSearchSpheresWithIdentifier;
}

void add_weight_from_search_result_to_map(RowMap& aRowMap,
                                          const RowSphereGlobalID& aSphereID,
                                          const ColumnNodeGlobalID& aNodeId,
                                          const Weight aWeight,
                                          const unsigned int aMaximumSize)
{
    auto& tEntry = aRowMap[aSphereID.mValue];
    tEntry.mRowSum += aWeight.mValue;
    tEntry.mNonzeroColumnGlobalIDs.reserve(aMaximumSize);
    tEntry.mNonzeroColumnGlobalIDs.push_back(aNodeId.mValue);
    tEntry.mColumnEntryWeights.reserve(aMaximumSize);
    tEntry.mColumnEntryWeights.push_back(aWeight.mValue);
}

void normalize_vector(std::vector<double>& aVector, const double aNormalization)
{
    assert(aNormalization > 0);

    std::transform(aVector.begin(), aVector.end(), aVector.begin(),
                   [&, aNormalization](const auto aEntry) { return aEntry / aNormalization; });
}

void normalize_rows_in_map(RowMap& aRowMap)
{
    for (auto& tRow : aRowMap)
    {
        normalize_vector(tRow.second.mColumnEntryWeights, tRow.second.mRowSum);
    }
}

unsigned int reduce_search_result_size(const third_party_integration::stk_search::SearchResults& aSearchResults,
                                       const boost::mpi::communicator& aCommunicator)
{
    unsigned int tResultSize = aSearchResults.size();
    boost::mpi::all_reduce(aCommunicator, boost::mpi::inplace(tResultSize), std::plus<unsigned int>());
    return tResultSize;
}

}  // namespace detail

}  // namespace plato::filter::extension
