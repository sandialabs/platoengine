#include "plato/filter/extension/kernel_filters/LinearMaskBuilderUtilities.hpp"

#include <assert.h>

#include <algorithm>
#include <boost/mpi/collectives.hpp>
#include <boost/mpi/collectives/broadcast.hpp>
#include <boost/mpi/communicator.hpp>
#include <numbers>
#include <ranges>

#include "plato/utilities/RankSplitVector.hpp"

namespace plato::filter::extension::kernel_filters
{
namespace
{
constexpr bool kZeroOut = true;
}

namespace detail
{

auto stk_search_points(const third_party_integration::tpetra::TpetraMultiVector& aNodalCoordinates, const int aRank)
    -> std::vector<third_party_integration::stk_search::SearchPointWithIdentifier>
{
    namespace tpi = third_party_integration;

    const auto tNumberOfLocalElements = tpi::tpetra::number_of_local_elements(aNodalCoordinates.getMap());
    auto tLocalSearchPointWithIdentifiers =
        std::vector<tpi::stk_search::SearchPointWithIdentifier>(tNumberOfLocalElements);

    for (const auto tLocalIndex : std::views::iota(0, tNumberOfLocalElements))
    {
        const auto tGlobalID = aNodalCoordinates.getMap()->getGlobalElement(tLocalIndex);
        const tpi::stk_search::Identifier tIdentifier{tGlobalID, aRank};
        const tpi::common::Coordinate tCoordinate = tpi::tpetra::multivector_coordinate(aNodalCoordinates, tLocalIndex);
        tLocalSearchPointWithIdentifiers[tLocalIndex] =
            tpi::stk_search::SearchPointWithIdentifier({tpi::stk_search::convert_coordinate(tCoordinate), tIdentifier});
    }
    return tLocalSearchPointWithIdentifiers;
}

auto distribute_and_label_search_vector(const SourceColumnVector& aSourceColumnVector,
                                        const boost::mpi::communicator& aCommunicator)
    -> std::vector<third_party_integration::stk_search::SearchPointWithIdentifier>
{
    third_party_integration::tpetra::TpetraMultiVector tDistributedNodalCoordinates(
        detail::create_tpetravector_coordinates(aSourceColumnVector.mValue, aCommunicator));
    return detail::stk_search_points(tDistributedNodalCoordinates, aCommunicator.rank());
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

void add_weight_from_search_result_to_map(RowMap& aRowMap,
                                          const RowSearchEntityGlobalID& aSphereID,
                                          const ColumnNodeGlobalID& aNodeId,
                                          const Weight aWeight,
                                          const unsigned int aMaximumSize)
{
    auto& tEntry = aRowMap[aSphereID.mValue];

    if (std::find(tEntry.mNonzeroColumnGlobalIDs.begin(), tEntry.mNonzeroColumnGlobalIDs.end(), aNodeId.mValue) ==
        tEntry.mNonzeroColumnGlobalIDs.end())
    {
        tEntry.mRowSum += aWeight.mValue;
        tEntry.mNonzeroColumnGlobalIDs.reserve(aMaximumSize);
        tEntry.mNonzeroColumnGlobalIDs.push_back(aNodeId.mValue);
        tEntry.mColumnEntryWeights.reserve(aMaximumSize);
        tEntry.mColumnEntryWeights.push_back(aWeight.mValue);
    }
}

unsigned int reduce_search_result_size(const third_party_integration::stk_search::SearchResults& aSearchResults,
                                       const boost::mpi::communicator& aCommunicator)
{
    unsigned int tResultSize = aSearchResults.size();
    boost::mpi::all_reduce(aCommunicator, boost::mpi::inplace(tResultSize), std::plus<unsigned int>());
    return tResultSize;
}

boost::mpi::communicator subdivide_world_comm_into_groups(const unsigned int aGroupSize)
{
    auto tWorldComm = boost::mpi::communicator{};

    const auto tVectorSize = tWorldComm.size() / aGroupSize;
    const std::vector<unsigned int> tGroups(tVectorSize, aGroupSize);
    const auto tRank = tWorldComm.rank();
    const auto tColor = utilities::rank_group_color(tGroups, utilities::RankNamedType{tRank});
    return tWorldComm.split(tColor.mValue);
}

auto number_of_column_entries_per_row(const RowMap& aRowMap) -> std::vector<std::size_t>
{
    auto tGlobalIDs = std::vector<third_party_integration::tpetra::TpetraGlobalOrdinal>{};
    tGlobalIDs.reserve(aRowMap.size());
    std::ranges::copy(std::views::keys(aRowMap), std::back_inserter(tGlobalIDs));
    std::ranges::sort(tGlobalIDs);

    auto tNumberOfColumnsPerRow = std::vector<std::size_t>{};
    tNumberOfColumnsPerRow.reserve(aRowMap.size());
    std::ranges::transform(tGlobalIDs, std::back_inserter(tNumberOfColumnsPerRow), [&aRowMap](const auto tGlobalID)
                           { return aRowMap.at(tGlobalID).mNonzeroColumnGlobalIDs.size(); });

    return tNumberOfColumnsPerRow;
}

}  // namespace detail

}  // namespace plato::filter::extension::kernel_filters
