#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_VOLUMEUTILITIES
#define PLATO_THIRDPARTYINTEGRATION_STKIO_VOLUMEUTILITIES

#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Entity.hpp>

#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::third_party_integration::stk_io
{
using ElementNodalSensitivities =
    utilities::NamedType<std::vector<common::Vector3>, struct ElementNodalSensitivitiesTag>;

/// @brief Returns the volume of an element @a Element found in bulk data @a aBulk
[[nodiscard]] auto element_volume(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk) -> double;

/// @brief Returns the centroid of an element @a Element found in bulk data @a aBulk
[[nodiscard]] auto element_centroid(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk)
    -> common::Coordinate;

/// @brief Returns a vector containing the sensitivities of the volume of an element @a aElement in bulk data @a aBulk
/// w.r.t. each nodal coordinate
[[nodiscard]] auto element_volume_nodal_sensitivities(const stk::mesh::Entity& aElement,
                                                      const stk::mesh::BulkData& aBulk) -> ElementNodalSensitivities;

/// @brief Returns the maximum edge length of an element @a Element found in bulk data @a aBulk
[[nodiscard]] auto element_max_edge_length(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk)
    -> double;

/// @brief Return a vector of all the element centroids found in bulk data @a aBulk
[[nodiscard]] auto element_centroids(const stk::mesh::BulkData& aBulk) -> std::vector<common::Coordinate>;

/// @brief Returns a vector of the element centroids associated with the parts given in @a aParts.
[[nodiscard]] auto element_centroids(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts)
    -> std::vector<common::Coordinate>;

/// @brief Returns the total volume of a mesh found in bulk data @a aBulk
[[nodiscard]] auto mesh_volume(const stk::mesh::BulkData& aBulk) -> double;

/// @brief Returns a vector containing the nodal sensitivities of the volume of domains @a aParts in a mesh represented
/// by bulk data @a aBulk
/// @post The sensitivities are stored in a std::vector that is sorted by global node ID in ascending order.
[[nodiscard]] auto volume_nodal_sensitivities(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts)
    -> std::vector<third_party_integration::common::Vector3>;

/// @brief Returns a vector containing the coordinates of an element @a aElement in bulk data @a aBulk
[[nodiscard]] auto element_coordinates(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk)
    -> std::vector<common::Coordinate>;

/// @brief Returns the average element maximum edge length over all the elements found in bulk data @a aBulk
[[nodiscard]] auto average_element_max_edge_length(const stk::mesh::BulkData& aBulk) -> double;

}  // namespace plato::third_party_integration::stk_io

#endif
