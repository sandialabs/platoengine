#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_LINEARMASKBUILDERUTILITIES
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_LINEARMASKBUILDERUTILITIES

#include <boost/mpi/communicator.hpp>
#include <vector>

#include "plato/mesh/Mesh.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/stk_search/Utilities.hpp"
#include "plato/third_party_integration/tpetra/Utilities.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::filter::extension::kernel_filters
{
using SourceMesh = utilities::NamedType<mesh::Mesh, struct SourceMeshTag>;
using TargetMesh = utilities::NamedType<mesh::Mesh, struct TargetMeshTag>;
using SourcePoint = utilities::NamedType<third_party_integration::common::Coordinate, struct SourcePointTag>;
using TargetPoint = utilities::NamedType<third_party_integration::common::Coordinate, struct TargetPointTag>;

using Distance = utilities::NamedType<double, struct DistanceTag>;
using RowSearchEntityGlobalID =
    utilities::NamedType<third_party_integration::tpetra::TpetraGlobalOrdinal, struct RowSearchEntityGlobalIDTag>;
using ColumnNodeGlobalID =
    utilities::NamedType<third_party_integration::tpetra::TpetraGlobalOrdinal, struct ColumnNodeGlobalIDTag>;

using Weight = utilities::NamedType<double, struct WeightTag>;

struct RowDetail
{
    std::vector<third_party_integration::tpetra::TpetraGlobalOrdinal> mNonzeroColumnGlobalIDs;
    std::vector<double> mColumnEntryWeights;
    double mRowSum = 0;
};
using RowMap = std::unordered_map<third_party_integration::tpetra::TpetraGlobalOrdinal, RowDetail>;

using SourceColumnVector =
    utilities::NamedType<std::vector<third_party_integration::common::Coordinate>, struct SourceColumnVectorTag>;
using TargetRowVector =
    utilities::NamedType<std::vector<third_party_integration::common::Coordinate>, struct TargetRowVectorTag>;

namespace detail
{
/// @brief Helper function to take a multivector @a aNodalCoordinates and a given rank @a aRank will generate the STK
/// formatted Search point equal to the node and assigned a local id and rank.
[[nodiscard]] auto stk_search_points(const third_party_integration::tpetra::TpetraMultiVector& aNodalCoordinates,
                                     const int aRank)
    -> std::vector<third_party_integration::stk_search::SearchPointWithIdentifier>;

/// @brief Helper function to take a nodal vector @a aSourceColumnVector and the communicator @a aCommunicator and
/// distribute the vector and give each point a unique stk search identifier
[[nodiscard]] auto distribute_and_label_search_vector(const SourceColumnVector& aSourceColumnVector,
                                                      const boost::mpi::communicator& aCommunicator)
    -> std::vector<third_party_integration::stk_search::SearchPointWithIdentifier>;

/// @brief create nodal coordinate tpetra container of  @a aCoordinates
[[nodiscard]] auto create_tpetravector_coordinates(
    const std::vector<third_party_integration::common::Coordinate>& aCoordinates,
    const boost::mpi::communicator& aCommunicator) -> third_party_integration::tpetra::TpetraMultiVector;

/// @brief Transform entries in vector @a aVector by dividing by @a aNormalization
///
/// @pre @a aNormalization is > 0
void normalize_vector(std::vector<double>& aVector, const double aNormalization);

/// @brief Iterate through the row map @a aRowMap and normalize the detail struct so that the entires sum to 1.
void normalize_rows_in_map(RowMap& aRowMap);

/// @brief Take a search result pair given by a row sphere @a aSphereID and a column node @a aNodeID and determine their
/// weighting function. Enter this weight into the row map @a RowMap.
void add_weight_from_search_result_to_map(RowMap& aRowMap,
                                          const RowSearchEntityGlobalID& aSphereID,
                                          const ColumnNodeGlobalID& aNodeID,
                                          const Weight aWeight,
                                          const unsigned int aMaximumSize);

/// @brief Tally up the size of search results @a aSearchResults across the comms in @a aCommunicator
[[nodiscard]] unsigned int reduce_search_result_size(
    const third_party_integration::stk_search::SearchResults& aSearchResults,
    const boost::mpi::communicator& aCommunicator);

[[nodiscard]] auto subdivide_world_comm_into_groups(const unsigned int aGroupSize) -> boost::mpi::communicator;

/// @brief Creates a vector containing the number of columns in each row, sorted by global row ID.
///
/// The purpose of this function is to provide the maximum number of entries to allocate in a Tpetra::CRSMatrix.
[[nodiscard]] auto number_of_column_entries_per_row(const RowMap& aRowMap) -> std::vector<std::size_t>;

}  // namespace detail

}  // namespace plato::filter::extension::kernel_filters

#endif
