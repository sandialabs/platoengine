#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_CYLINDRICALMASKUTILITIES
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_CYLINDRICALMASKUTILITIES

#include "plato/filter/extension/kernel_filters/LinearMaskBuilderUtilities.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::mesh
{
class Mesh;
}

namespace plato::filter::extension::kernel_filters
{
using CylinderRadius = utilities::NamedType<double, struct CylinderRadiusTag>;
using CylinderAxis = utilities::NamedType<third_party_integration::common::Vector3, struct CylinderAxisTag>;

namespace detail
{

using CylinderCenter = utilities::NamedType<third_party_integration::common::Coordinate, struct CylinderCenterTag>;
using CylinderLength = utilities::NamedType<double, struct CylinderRadiusTag>;
using QueryLocation = utilities::NamedType<third_party_integration::common::Coordinate, struct QueryLocationTag>;
using CylindricalPolarCoordinate =
    utilities::NamedType<third_party_integration::common::Coordinate, struct CylindricalPolarCoordinateTag>;
using CylindricalPolarCoordinates = utilities::NamedType<std::vector<third_party_integration::common::Coordinate>,
                                                         struct CylindricalPolarCoordinatesTag>;

/// @brief Computes the linear top hat function based on an infinitely long cylinder defined with center @a
/// aCylinderCenter and axis @a aCylinderAxis with radius @a aCylinderRadius to a query point @a aQueryLocation.
[[nodiscard]] double cylinder_ramp_weight(const CylinderCenter& aCylinderCenter,
                                          const CylinderAxis& aCylinderAxis,
                                          const CylinderRadius aCylinderRadius,
                                          const QueryLocation& aQueryLocation);

/// @brief Take a tpetra center vector @a aCenteringCoordinates and create stk_search objects that are search boxes with
/// a long dimension equal to the largest size of the domain and equal sides that are equal to the search radius @a
/// aRadius, oriented by specific axis @a aAxis with radii  and assigned the proper identifier based on the rank @a
/// aRank.
[[nodiscard]] auto stk_search_boxes(const third_party_integration::tpetra::TpetraMultiVector& aCenteringCoordinates,
                                    const CylinderRadius aRadius,
                                    const CylinderAxis& aAxis,
                                    const int aRank)
    -> std::vector<third_party_integration::stk_search::SearchBoxWithIdentifier>;

/// @brief Take a center vector @a aTargetRowVector and a nodal vector @a aSourceColumnVector and distribute them using
/// tpetra objects over the communicator provided @a aCommunicator. Then conduct a parallel STK search using the
/// cylinder defined by the cylinder center aN axis @a aCylinderAxis and a radius @a aCylinderRadius
/// @pre The cylinder axis is scaled so that it covers the domain, e.g. if min z = -5, and max z =5, and the axis is
/// aligned with z, then the axis = {0, 0, 10}. This cylinder axis is used to create a search box, the entries in the
/// tpetra matrix will have the cylinder ramp weights and that means some will be in the search box with 0 weight
[[nodiscard]] auto distribute_search_vectors_and_stk_search_with_cylinders(
    const TargetRowVector& aTargetRowVector,
    const SourceColumnVector& aSourceColumnVector,
    const CylinderAxis& aCylinderAxis,
    const CylinderRadius aCylinderRadius,
    const boost::mpi::communicator& aCommunicator) -> third_party_integration::stk_search::SearchResults;

///@brief Take a coordinate @a aCoordinate and convert it to cylindrical polar coordinates
[[nodiscard]] auto cylindrical_polar_coordinates(const third_party_integration::common::Coordinate& aCoordinate)
    -> CylindricalPolarCoordinate;

///@brief Take a vector of coordinates @a aCoordinates and convert each entry to cylindrical polar coordinates
[[nodiscard]] auto cylindrical_polar_coordinates(
    const std::vector<third_party_integration::common::Coordinate>& aCoordinates) -> CylindricalPolarCoordinates;

}  // namespace detail
}  // namespace plato::filter::extension::kernel_filters
#endif
