#include "plato/filter/extension/kernel_filters/LinearMaskBuilder.hpp"

#include <Teuchos_ArrayViewDecl.hpp>
#include <Teuchos_DefaultMpiComm.hpp>
#include <Teuchos_RCP.hpp>
#include <boost/mpi/communicator.hpp>
#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include "plato/filter/extension/kernel_filters/KernelFilterCenteringTypes.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilderUtilities.hpp"
#include "plato/filter/extension/kernel_filters/SphericalMaskUtilities.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/third_party_integration/stk_search/Utilities.hpp"
#include "plato/third_party_integration/tpetra/Utilities.hpp"

namespace plato::filter::extension::kernel_filters
{
LinearMaskBuilder::LinearMaskBuilder(const SourceMesh& aSourceMesh,
                                     const TargetMesh& aTargetMesh,
                                     const KernelFilterFunctions& aFilterType,
                                     const boost::mpi::communicator& aCommunicator)
    : mCommunicator(aCommunicator),
      mGlobalRowCenterCoordinates(detail::center_coordinates(aTargetMesh.mValue, aFilterType.mFilterCentering)),
      mGlobalColumnCenterCoordinates(mesh::EntityRetrieval{aSourceMesh.mValue}.designDomainNodalCoordinates()),
      mMaximumConnectivityEstimate(
          detail::average_nodes_in_filter_radius_estimate(aSourceMesh.mValue, SearchRadius{aFilterType.mRadius})),
      mFilterType(aFilterType)
{
    generateDistanceMap();
}

LinearMaskBuilder::LinearMaskBuilder(const SourceColumnVector& aNodalCoordinates,
                                     TargetRowVector aCenters,
                                     const KernelFilterFunctions& aFilterType,
                                     const int aMaximumConnectivityEstimate,
                                     const boost::mpi::communicator& aCommunicator)
    : mCommunicator(aCommunicator),
      mGlobalRowCenterCoordinates(std::move(aCenters.mValue)),
      mGlobalColumnCenterCoordinates(aNodalCoordinates.mValue),
      mMaximumConnectivityEstimate(aMaximumConnectivityEstimate),
      mFilterType(aFilterType)
{
    generateDistanceMap();
}

auto LinearMaskBuilder::mask() const -> const third_party_integration::tpetra::TpetraCRSMatrix& { return *mLinearMask; }

void LinearMaskBuilder::create_linear_mask(const RowMap& aRowMap)
{
    namespace tpi = third_party_integration;
    const auto tCommunicator = Teuchos::rcp(std::make_unique<Teuchos::MpiComm<int>>(mCommunicator).release());
    auto tCrsRowMap = Teuchos::rcp(std::make_unique<tpi::tpetra::TpetraMap>(mGlobalRowCenterCoordinates.size(),
                                                                            tpi::tpetra::kIndexBase, tCommunicator)
                                       .release());
    auto tCrsDomainMap = Teuchos::rcp(std::make_unique<tpi::tpetra::TpetraMap>(mGlobalColumnCenterCoordinates.size(),
                                                                               tpi::tpetra::kIndexBase, tCommunicator)
                                          .release());

    mLinearMask = std::make_unique<tpi::tpetra::TpetraCRSMatrix>(
        tCrsRowMap, Teuchos::ArrayView<const std::size_t>{detail::number_of_column_entries_per_row(aRowMap)});

    for (const auto& tRow : aRowMap)
    {
        const auto tRowGlobalID = tRow.first;
        const auto& tColumnGlobalIDs = aRowMap.at(tRowGlobalID).mNonzeroColumnGlobalIDs;
        const auto& tColumnWeights = aRowMap.at(tRowGlobalID).mColumnEntryWeights;

        assert(!tColumnGlobalIDs.empty());

        mLinearMask->insertGlobalValues(tRowGlobalID,
                                        Teuchos::ArrayView<const tpi::tpetra::TpetraGlobalOrdinal>(tColumnGlobalIDs),
                                        Teuchos::ArrayView<const tpi::tpetra::TpetraScalar>(tColumnWeights));
    }

    mLinearMask->fillComplete(tCrsDomainMap, tCrsRowMap);
}

RowMap LinearMaskBuilder::create_normalized_row_map(
    const third_party_integration::stk_search::SearchResults& aSearchResults)
{
    RowMap tRowMap;
    tRowMap.reserve(mGlobalRowCenterCoordinates.size());
    for (const auto& tSearchResult : aSearchResults)
    {
        const auto tSphereID = RowSearchEntityGlobalID{tSearchResult.first.id()};
        const auto tNodeId = ColumnNodeGlobalID{tSearchResult.second.id()};
        const double tWeight = unnormalized_weight(tSphereID, tNodeId);
        detail::add_weight_from_search_result_to_map(tRowMap, tSphereID, tNodeId, Weight{tWeight},
                                                     mMaximumConnectivityEstimate);
    }
    detail::normalize_rows_in_map(tRowMap);
    return tRowMap;
}

double LinearMaskBuilder::unnormalized_weight(const RowSearchEntityGlobalID aSphereID, const ColumnNodeGlobalID aNodeID)
{
    const auto tSphereCoordinate = mGlobalRowCenterCoordinates[aSphereID.mValue];
    const auto tNodalCoordinate = mGlobalColumnCenterCoordinates[aNodeID.mValue];

    return mFilterType.mUnnormalizedWeightFunction(SourcePoint{tNodalCoordinate}, TargetPoint{tSphereCoordinate});
}

void LinearMaskBuilder::generateDistanceMap()
{
    const auto tSearchResults =
        mFilterType.mSearchFunction(TargetRowVector{mGlobalRowCenterCoordinates},
                                    SourceColumnVector{mGlobalColumnCenterCoordinates}, mCommunicator);

    assert(detail::reduce_search_result_size(tSearchResults, mCommunicator) >= mGlobalRowCenterCoordinates.size());

    const auto tRowMap = create_normalized_row_map(tSearchResults);
    create_linear_mask(tRowMap);
}

namespace detail
{
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
}  // namespace detail

}  // namespace plato::filter::extension::kernel_filters
