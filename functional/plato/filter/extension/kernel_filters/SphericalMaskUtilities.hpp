#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_SPHERICALMASKUTILITIES
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_SPHERICALMASKUTILITIES

#include <boost/mpi/communicator.hpp>

#include "plato/filter/extension/kernel_filters/LinearMaskBuilderUtilities.hpp"
#include "plato/filter/extension/kernel_filters/Plane.hpp"
#include "plato/filter/extension/kernel_filters/ReflectionUtilities.hpp"
#include "plato/third_party_integration/stk_search/Utilities.hpp"
#include "plato/third_party_integration/tpetra/Utilities.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::mesh
{
class Mesh;
}

namespace plato::filter::extension::kernel_filters
{

using SearchRadius = utilities::NamedType<double, struct SearchRadiusTag>;

/// @brief Take a center vector @a aTargetRowVector and a nodal vector @a aSourceColumnVector and distribute them using
/// tpetra objects over the communicator provided @a aCommunicator. Then conduct a parallel STK search using a search
/// radius @a aSearchRadius. Return the stk search results.
[[nodiscard]] auto distribute_search_vectors_and_stk_search_with_spheres(const TargetRowVector& aTargetRowVector,
                                                                         const SourceColumnVector& aSourceColumnVector,
                                                                         const double aSearchRadius,
                                                                         const boost::mpi::communicator& aCommunicator)
    -> third_party_integration::stk_search::SearchResults;

/// @brief Using a source vector of points @a aSourceColumnVector and a target set @a aTargetRowVector, determine what
/// target points map back to a source point under the mirrored wedge reflection defined in @a aWedge. A search sphere
/// of radius @a aRadius is used in conjunction with the reflections
[[nodiscard]] auto distribute_search_vectors_and_stk_search_with_spheres(const TargetRowVector& aTargetRowVector,
                                                                         const SourceColumnVector& aSourceColumnVector,
                                                                         const double aRadius,
                                                                         const boost::mpi::communicator& aCommunicator,
                                                                         const Wedge& aWedge)
    -> third_party_integration::stk_search::SearchResults;

/// @brief Using a source vector of points @a aSourceColumnVector and a target set @a aTargetRowVector, determine what
/// target points map back to a source point under the reflection list. A search sphere of radius @a aRadius is used in
/// conjunction with the reflections
template <typename... ReflectionPlanes>
[[nodiscard]] auto distribute_search_vectors_and_stk_search_with_spheres(const TargetRowVector& aTargetRowVector,
                                                                         const SourceColumnVector& aSourceColumnVector,
                                                                         const double aRadius,
                                                                         const boost::mpi::communicator& aCommunicator,
                                                                         const ReflectionPlanes&... aPlanes)
    -> third_party_integration::stk_search::SearchResults;

namespace detail
{

using RowSum = utilities::NamedType<double, struct RowSumTag>;
using EstimatedConnectivity = utilities::NamedType<unsigned int, struct EstimatedConnectivityTag>;

/// @brief Computes the linear top hat function based on a distance @a aDistance and a search radius @a aSearchRadius
[[nodiscard]] double linear_ramp_weight(const Distance aDistance, const SearchRadius aSearchRadius);

/// @brief Compute the volume of a sphere with radius @a aFilterRadius
[[nodiscard]] double filter_volume(const SearchRadius aSearchRadius);

/// @brief Compute the area of a circle with radius @a aFilterRadius
[[nodiscard]] double filter_area(const SearchRadius aSearchRadius);

/// @brief Take a tpetra center vector @a aCenteringCoordinates and create stk_search objects that are search spheres
/// with radii @a aRadius and assigned the proper identifier based on the rank @a aRank.
[[nodiscard]] auto stk_search_spheres(const third_party_integration::tpetra::TpetraMultiVector& aCenteringCoordinates,
                                      const SearchRadius aRadius,
                                      const int aRank)
    -> std::vector<third_party_integration::stk_search::SearchSphereWithIdentifier>;

/// @brief Estimates the the number of nodes within a filter radius @a aFilterRadius based on the average nodal
/// connectivity of @a aMesh.
///
/// The purpose of this function is to provide an estimate for allocating memory for the kernel filter.
[[nodiscard]] auto average_nodes_in_filter_radius_estimate(const mesh::Mesh& aMesh, SearchRadius aFilterRadius) -> int;

}  // namespace detail

template <typename... ReflectionPlanes>
auto distribute_search_vectors_and_stk_search_with_spheres(const TargetRowVector& aTargetRowVector,
                                                           const SourceColumnVector& aSourceColumnVector,
                                                           const double aRadius,
                                                           const boost::mpi::communicator& aCommunicator,
                                                           const ReflectionPlanes&... aPlanes)
    -> third_party_integration::stk_search::SearchResults
{
    namespace tpi = third_party_integration;
    const auto tLocalSearchPointWithIdentifiers =
        detail::distribute_and_label_search_vector(aSourceColumnVector, aCommunicator);

    const auto tReflectedSearchPointsWithIdentifiers =
        detail::reflect_points_in_planes(tLocalSearchPointWithIdentifiers, aPlanes...);

    const tpi::tpetra::TpetraMultiVector tLocalRowCenterCoordinates =
        detail::create_tpetravector_coordinates(aTargetRowVector.mValue, aCommunicator);
    const auto tLocalSearchSpheresWithIdentifiers =
        detail::stk_search_spheres(tLocalRowCenterCoordinates, SearchRadius{aRadius}, aCommunicator.rank());

    return tpi::stk_search::stk_search(tLocalSearchSpheresWithIdentifiers, tReflectedSearchPointsWithIdentifiers,
                                       aCommunicator);
}
}  // namespace plato::filter::extension::kernel_filters

#endif
