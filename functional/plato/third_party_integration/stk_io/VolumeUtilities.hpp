#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_VOLUMEUTILITIES
#define PLATO_THIRDPARTYINTEGRATION_STKIO_VOLUMEUTILITIES

#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Entity.hpp>

#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"

namespace plato::third_party_integration::stk_io
{

/// @brief Returns the volume of an element @a Element found in bulk data @a aBulk
[[nodiscard]] double element_volume(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk);

/// @brief Returns the centroid of an element @a Element found in bulk data @a aBulk
[[nodiscard]] common::Coordinate element_centroid(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk);

/// @brief Returns the maximum edge length of an element @a Element found in bulk data @a aBulk
[[nodiscard]] double element_max_edge_length(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk);

/// @brief Return a vector of all the element centroids found in bulk data @a aBulk
[[nodiscard]] std::vector<common::Coordinate> element_centroids(const stk::mesh::BulkData& aBulk);

/// @brief Returns a vector of the element centroids associated with the parts given in @a aParts.
[[nodiscard]] std::vector<common::Coordinate> element_centroids(const stk::mesh::BulkData& aBulk,
                                                                const PartReferenceVector& aParts);

/// @brief Returns the total volume of a mesh found in bulk data @a aBulk
[[nodiscard]] double mesh_volume(const stk::mesh::BulkData& aBulk);

/// @brief Returns a vector containing the coordinates of an element @a aElement in bulk data @a aBulk
[[nodiscard]] std::vector<common::Coordinate> element_coordinates(const stk::mesh::Entity& aElement,
                                                                  const stk::mesh::BulkData& aBulk);

/// @brief Returns the average element maximum edge length over all the elements found in bulk data @a aBulk
[[nodiscard]] double average_element_max_edge_length(const stk::mesh::BulkData& aBulk);

}  // namespace plato::third_party_integration::stk_io

#endif
