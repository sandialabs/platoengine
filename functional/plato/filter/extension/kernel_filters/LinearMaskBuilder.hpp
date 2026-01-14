#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_LINEARMASKBUILDER
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_LINEARMASKBUILDER

#include <boost/mpi/communicator.hpp>
#include <functional>
#include <memory>

#include "plato/filter/extension/kernel_filters/KernelFilterCenteringTypes.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilderUtilities.hpp"
#include "plato/filter/extension/kernel_filters/SphericalMaskUtilities.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/third_party_integration/stk_search/Utilities.hpp"
#include "plato/third_party_integration/tpetra/Utilities.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::mesh
{
class Mesh;
}

namespace plato::filter::extension::kernel_filters
{

/// @brief a struct which defines the key pieces of a kernel filter. The SearchFunction and UnnormalizedWeightFunction
/// are the key pieces that define a symmetry filter by defining relationships between the source and target mesh and
/// how to populate the weights in a row of the kernel filter.
template <typename InputParserType>
struct KernelFilterFunctions
{
    /// @brief a function that takes target and source vectors returns specialized search results
    using SearchFunction = std::function<third_party_integration::stk_search::SearchResults(
        const TargetRowVector&, const SourceColumnVector&, const boost::mpi::communicator&)>;

    /// @brief a function that returns a weight for a given target and source point
    using UnnormalizedWeightFunction = std::function<double(const SourcePoint&, const TargetPoint&)>;

    /// @brief a function that returns the target mesh
    using TargetMeshFunction = std::function<mesh::Mesh(const InputParserType&)>;

    double mRadius;
    input_parser::KernelFilterCenteringTypes mFilterCentering;
    SearchFunction mSearchFunction;
    UnnormalizedWeightFunction mUnnormalizedWeightFunction;
    TargetMeshFunction mTargetMeshFunction;
};

using SourceMesh = utilities::NamedType<mesh::Mesh, struct SourceMeshTag>;
using TargetMesh = utilities::NamedType<mesh::Mesh, struct TargetMeshTag>;
/// @brief A mask generation class for the filters that are based on a linear mask, kernel filter and symmetry filters.
///
/// The intent is that this object gets created during some initialization phase and does not get updated thereafter.
template <typename InputParserType>
class LinearMaskBuilder
{
   public:
    /// @brief Construction from a source mesh @a aSourceMesh and @a aTargetMesh, given the filter info @a aFilterInfo
    ///
    /// This will create a filter that expects a nodal field defined at all nodes and will filter to either nodes or
    /// element centroids as given by @a aCenteringType.
    LinearMaskBuilder(const SourceMesh& aSourceMesh,
                      const TargetMesh& aTargetMesh,
                      input_parser::KernelFilterCenteringTypes aCenteringType,
                      const KernelFilterFunctions<InputParserType>& aFilterType,
                      const boost::mpi::communicator& aCommunicator);

    /// @brief Construction directly from vectors of points.
    ///
    /// @param aNodalCoordinates is a vector of nodal coordinates, presumably from the mesh directly.
    /// @param aCenters is a vector of the element centroids if it is element centered or nodes if it is nodal.
    /// @param aFilterInfo all the information required to create this mask
    /// @param aMaximumConnectivityEstimate is an estimate provided to the Tpetra CRS Matrix during allocation. It
    /// should be a maximum expected to avoid any additional allocation time.
    LinearMaskBuilder(const SourceColumnVector& aNodalCoordinates,
                      TargetRowVector aCenters,
                      const KernelFilterFunctions<InputParserType>& aFilterType,
                      const int aMaximumConnectivityEstimate,
                      const boost::mpi::communicator& aCommunicator);
    virtual ~LinearMaskBuilder() = default;

    /// @brief Return a reference to the distance mask.
    [[nodiscard]] auto mask() const -> const third_party_integration::tpetra::TpetraCRSMatrix&;

    LinearMaskBuilder(const LinearMaskBuilder&) = delete;
    LinearMaskBuilder& operator=(const LinearMaskBuilder&) = delete;
    LinearMaskBuilder(LinearMaskBuilder&&) = delete;
    LinearMaskBuilder& operator=(LinearMaskBuilder&&) = delete;

   private:
    /// @brief Perform the calculation of the full NxM distance map.
    void generateDistanceMap();

    /// @brief Given a sphere ID @a aSphereID and a node entry @a aNodeID call the detail functions to determine the
    /// weight with the filter parameters
    [[nodiscard]] double unnormalized_weight(const RowSearchEntityGlobalID aSphereID, const ColumnNodeGlobalID aNodeID);

    /// @brief Take the RowMap data structure @a aRowMap and convert it to a CRS matrix
    void create_linear_mask(const RowMap& aRowMap);

    /// @brief Take the search results from STK @a aSearchResults and convert them to a RowMap, a map that takes Tpetra
    /// global ID and maps it to the details regarding that row (nonzero entries, their values, and their total)
    [[nodiscard]] RowMap create_normalized_row_map(
        const third_party_integration::stk_search::SearchResults& aSearchResults);

    boost::mpi::communicator mCommunicator;
    std::vector<third_party_integration::common::Coordinate> mGlobalRowCenterCoordinates;
    std::vector<third_party_integration::common::Coordinate> mGlobalColumnCenterCoordinates;
    unsigned int mMaximumConnectivityEstimate = 1;
    KernelFilterFunctions<InputParserType> mFilterType;
    std::unique_ptr<third_party_integration::tpetra::TpetraCRSMatrix> mLinearMask;
};

namespace detail
{
///@brief Given a mesh @a aMesh, return the nodal coordinates or the element centroids depending on the value of @a
/// aCenteringType
[[nodiscard]] auto center_coordinates(const mesh::Mesh& aMesh, input_parser::KernelFilterCenteringTypes aCenteringType)
    -> std::vector<third_party_integration::common::Coordinate>;

}  // namespace detail

template <typename InputParserType>
LinearMaskBuilder<InputParserType>::LinearMaskBuilder(const SourceMesh& aSourceMesh,
                                                      const TargetMesh& aTargetMesh,
                                                      input_parser::KernelFilterCenteringTypes aCenteringType,
                                                      const KernelFilterFunctions<InputParserType>& aFilterType,
                                                      const boost::mpi::communicator& aCommunicator)
    : mCommunicator(aCommunicator),
      mGlobalRowCenterCoordinates(detail::center_coordinates(aTargetMesh.mValue, aCenteringType)),
      mGlobalColumnCenterCoordinates(mesh::EntityRetrieval{aSourceMesh.mValue}.designDomainNodalCoordinates()),
      mMaximumConnectivityEstimate(
          detail::average_nodes_in_filter_radius_estimate(aSourceMesh.mValue, SearchRadius{aFilterType.mRadius})),
      mFilterType(aFilterType)
{
    generateDistanceMap();
}

template <typename InputParserType>
LinearMaskBuilder<InputParserType>::LinearMaskBuilder(const SourceColumnVector& aNodalCoordinates,
                                                      TargetRowVector aCenters,
                                                      const KernelFilterFunctions<InputParserType>& aFilterType,
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

template <typename InputParserType>
auto LinearMaskBuilder<InputParserType>::mask() const -> const third_party_integration::tpetra::TpetraCRSMatrix&
{
    return *mLinearMask;
}

template <typename InputParserType>
void LinearMaskBuilder<InputParserType>::create_linear_mask(const RowMap& aRowMap)
{
    namespace tpi = third_party_integration;
    const auto tCommunicator = Teuchos::rcp(new Teuchos::MpiComm<int>(mCommunicator));
    auto tCrsRowMap = Teuchos::rcp(
        new tpi::tpetra::TpetraMap(mGlobalRowCenterCoordinates.size(), tpi::tpetra::kIndexBase, tCommunicator));
    auto tCrsDomainMap = Teuchos::rcp(
        new tpi::tpetra::TpetraMap(mGlobalColumnCenterCoordinates.size(), tpi::tpetra::kIndexBase, tCommunicator));

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

template <typename InputParserType>
RowMap LinearMaskBuilder<InputParserType>::create_normalized_row_map(
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

template <typename InputParserType>
double LinearMaskBuilder<InputParserType>::unnormalized_weight(const RowSearchEntityGlobalID aSphereID,
                                                               const ColumnNodeGlobalID aNodeID)
{
    const auto tSphereCoordinate = mGlobalRowCenterCoordinates[aSphereID.mValue];
    const auto tNodalCoordinate = mGlobalColumnCenterCoordinates[aNodeID.mValue];

    return mFilterType.mUnnormalizedWeightFunction(SourcePoint{tNodalCoordinate}, TargetPoint{tSphereCoordinate});
}

template <typename InputParserType>
void LinearMaskBuilder<InputParserType>::generateDistanceMap()
{
    const auto tSearchResults =
        mFilterType.mSearchFunction(TargetRowVector{mGlobalRowCenterCoordinates},
                                    SourceColumnVector{mGlobalColumnCenterCoordinates}, mCommunicator);

    assert(detail::reduce_search_result_size(tSearchResults, mCommunicator) >= mGlobalRowCenterCoordinates.size());

    const auto tRowMap = create_normalized_row_map(tSearchResults);
    create_linear_mask(tRowMap);
}

}  // namespace plato::filter::extension::kernel_filters
#endif
