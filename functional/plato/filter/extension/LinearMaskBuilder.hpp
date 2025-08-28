#ifndef PLATO_FILTER_EXTENSION_LINEARMASKBUILDER
#define PLATO_FILTER_EXTENSION_LINEARMASKBUILDER

#include <boost/mpi/communicator.hpp>
#include <memory>

#include "plato/filter/extension/KernelFilterCenteringTypes.hpp"
#include "plato/third_party_integration/stk_search/Utilities.hpp"
#include "plato/third_party_integration/tpetra/Utilities.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::mesh
{
class Mesh;
}

namespace plato::filter::extension
{

using RowSphereGlobalID =
    utilities::NamedType<third_party_integration::tpetra::TpetraGlobalOrdinal, struct RowSphereGlobalIDTag>;
using ColumnNodeGlobalID =
    utilities::NamedType<third_party_integration::tpetra::TpetraGlobalOrdinal, struct ColumnNodeGlobalIDTag>;

struct RowDetail
{
    std::vector<third_party_integration::tpetra::TpetraGlobalOrdinal> mNonzeroColumnGlobalIDs;
    std::vector<double> mColumnEntryWeights;
    double mRowSum = 0;
};

using RowMap = std::unordered_map<third_party_integration::tpetra::TpetraGlobalOrdinal, RowDetail>;

using SearchRadius = utilities::NamedType<double, struct SearchRadiusTag>;
using Distance = utilities::NamedType<double, struct DistanceTag>;
using Weight = utilities::NamedType<double, struct WeightTag>;

using NodalVector =
    utilities::NamedType<std::vector<third_party_integration::common::Coordinate>, struct NodalVectorTag>;
using CenterVector =
    utilities::NamedType<std::vector<third_party_integration::common::Coordinate>, struct CenterVectorTag>;

using TpetraGlobalOrdinalVector = std::vector<third_party_integration::tpetra::TpetraGlobalOrdinal>;
using TpetraScalarVector = std::vector<third_party_integration::tpetra::TpetraScalar>;

/// @brief A mask generation class for the Kernel filter.
///
/// The intent is that this object gets created during some initialization phase and does not get updated thereafter.
class LinearMaskBuilder
{
   public:
    /// @brief Construction from a mesh.
    ///
    /// This will create a filter that expects a nodal field defined at all nodes and will filter to either nodes or
    /// element centroids as given by @a aCenteringType.
    LinearMaskBuilder(const mesh::Mesh& aMesh,
                      input_parser::KernelFilterCenteringTypes aCenteringType,
                      const SearchRadius aSearchRadius,
                      const boost::mpi::communicator& aCommunicator);

    /// @brief Construction directly from vectors of points.
    ///
    /// @param aNodalCoordinates is a vector of nodal coordinates, presumably from the mesh directly.
    /// @param aCenters is a vector of the element centroids if it is element centered or nodes if it is nodal.
    /// @param aSearchRadius is the search radius the distance map will be calculated over. It is used in a STK search
    /// as well as the linear function to determine the weight.
    /// @param aMaximumConnectivityEstimate is an estimate provided to the Tpetra CRS Matrix during allocation. It
    /// should be a maximum expected to avoid any additional allocation time.
    LinearMaskBuilder(const NodalVector& aNodalCoordinates,
                      CenterVector aCenters,
                      const SearchRadius aSearchRadius,
                      const int aMaximumConnectivityEstimate,
                      const boost::mpi::communicator& aCommunicator);

    /// @brief Return a reference to the distance mask.
    [[nodiscard]] auto mask() const -> const third_party_integration::tpetra::TpetraCRSMatrix&;

   private:
    /// @brief Perform the calculation of the full NxM distance map.
    void generateDistanceMap();

    /// @brief Given a sphere ID @a aSphereID and a node entry @a aNodeID call the detail functions to determine the
    /// weight with the filter parameters
    [[nodiscard]] double unnormalized_weight(const RowSphereGlobalID aSphereID, const ColumnNodeGlobalID aNodeID);

    /// @brief Take the search results from STK @a aSearchResults and convert them to a RowMap, a map that takes Tpetra
    /// global ID and maps it to the details regarding that row (nonzero entries, their values, and their total)
    [[nodiscard]] RowMap create_normalized_row_map(
        const third_party_integration::stk_search::SearchResults& aSearchResults);

    /// @brief Take the RowMap data structure @a aRowMap and convert it to a CRS matrix
    void create_linear_mask(const RowMap& aRowMap);

   private:
    boost::mpi::communicator mCommunicator;

    double mSearchRadius = 1;
    unsigned int mMaximumConnectivityEstimate = 1;

    std::vector<third_party_integration::common::Coordinate> mGlobalRowCenterCoordinates;
    std::vector<third_party_integration::common::Coordinate> mGlobalNodalCoordinates;

    std::unique_ptr<third_party_integration::tpetra::TpetraCRSMatrix> mLinearMask;
};

namespace detail
{

using RowSum = utilities::NamedType<double, struct RowSumTag>;
using EstimatedConnectivity = utilities::NamedType<unsigned int, struct EstimatedConnectivityTag>;

/// @brief Computes the linear tophat function based on a distance @a aDistance and a search radius @a aSearchRadius
[[nodiscard]] double linear_ramp_weight(const Distance aDistance, const SearchRadius aSearchRadius);

/// @brief Compute the volume of a sphere with radius @a aFilterRadius
[[nodiscard]] double filter_volume(const SearchRadius aSearchRadius);

/// @brief Compute the area of a circle with radius @a aFilterRadius
[[nodiscard]] double filter_area(const SearchRadius aSearchRadius);

/// @brief Estimates the the number of nodes within a filter radius @a aFilterRadius based on the average nodal
/// connectivity of @a aMesh.
///
/// The purpose of this function is to provide an estimate for allocating memory for the kernel filter.
[[nodiscard]] auto average_nodes_in_filter_radius_estimate(const mesh::Mesh& aMesh, SearchRadius aFilterRadius) -> int;

/// @brief create nodal coordinate tpetra container of  @a aCoordinates
[[nodiscard]] auto create_tpetravector_coordinates(
    const std::vector<third_party_integration::common::Coordinate>& aCoordinates,
    const boost::mpi::communicator& aCommunicator) -> third_party_integration::tpetra::TpetraMultiVector;

/// @brief Take a center vector @a aCenterVector and a nodal vector @a aNodalVector and distribute them using tpetra
/// objects over the communicator provided @a aCommunicator. Then conduct a parallel STK search using a search radius @a
/// aSearchRadius. Return the stk search results.
[[nodiscard]] auto distribute_search_vectors_and_stk_search(const CenterVector& aCenterVector,
                                                            const NodalVector& aNodalVector,
                                                            const double aSearchRadius,
                                                            const boost::mpi::communicator& aCommunicator)
    -> third_party_integration::stk_search::SearchResults;

/// @brief Helper function to take a multivector @a aNodalCoordinates and a given rank @a aRank will generate the STK
/// formatted Search point equal to the node and assigned a local id and rank.
[[nodiscard]] auto stk_search_points(const third_party_integration::tpetra::TpetraMultiVector& aNodalCoordinates,
                                     const int aRank)
    -> std::vector<third_party_integration::stk_search::SearchPointWithIdentifier>;

/// @brief Take a tpetra center vector @a aCenteringCoordinates and create stk_search objects that are search spheres
/// with radii @a aRadius and assigned the proper identifier based on the rank @a aRank.
[[nodiscard]] auto stk_search_spheres(const third_party_integration::tpetra::TpetraMultiVector& aCenteringCoordinates,
                                      const SearchRadius aRadius,
                                      const int aRank)
    -> std::vector<third_party_integration::stk_search::SearchSphereWithIdentifier>;

/// @brief Take a search result pair given by a row sphere @a aSphereID and a column node @a aNodeID and determine their
/// weighting function. Enter this weight into the row map @a RowMap.
void add_weight_from_search_result_to_map(RowMap& aRowMap,
                                          const RowSphereGlobalID& aSphereID,
                                          const ColumnNodeGlobalID& aNodeID,
                                          const Weight aWeight,
                                          const unsigned int aMaximumSize);

/// @brief Iterate through the row map @a aRowMap and normalize the detail struct so that the entires sum to 1.
void normalize_rows_in_map(RowMap& aRowMap);

/// @brief Tally up the size of search results @a aSearchResults across the comms in @a aCommunicator
[[nodiscard]] unsigned int reduce_search_result_size(
    const third_party_integration::stk_search::SearchResults& aSearchResults,
    const boost::mpi::communicator& aCommunicator);

void normalize_vector(std::vector<double>& aVector, const double aNormalization);

}  // namespace detail

}  // namespace plato::filter::extension

#endif
