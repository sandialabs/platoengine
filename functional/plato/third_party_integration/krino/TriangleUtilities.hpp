#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_TRIANGLEUTILITIES
#define PLATO_THIRDPARTYINTEGRATION_KRINO_TRIANGLEUTILITIES

#include <stk_mesh/base/Types.hpp>

#include "plato/third_party_integration/krino/SensitivityTriangle.hpp"

namespace plato::third_party_integration::krino
{

using PartReferenceVector = std::vector<std::reference_wrapper<const stk::mesh::Part>>;

/// @brief Get the name of the sideset defined between void and solid regions.
[[nodiscard]] auto interface_sideset_name() -> std::string;
/// @brief Given a triangle @ aTriangle calculate the normal nodal sensitivities.
[[nodiscard]] TriangleNormalSensitivity d_normal_d_tri_node(const SensitivityTriangle& aTriangle);
/// @brief Given a triangle @ aTriangle calculate the area nodal sensitivities.
[[nodiscard]] TriangleAreaSensitivity d_area_d_tri_node(const SensitivityTriangle& aTriangle);

namespace detail
{

/// @brief Given the nodal coordinates of a triangle @ aNodalCoords calculate the area sensitivities to changes in nodal
/// coordinates.
[[nodiscard]] std::vector<double> d_area_d_nodal_coords_from_tri_coords(const std::vector<double>& aNodalCoords);
/// @brief Given the nodal coordinates of a triangle @ aNodalCoords calculate the normal sensitivities to changes in
/// nodal coordinates.
[[nodiscard]] std::vector<double> d_normal_d_nodal_coords_from_tri_coords(const std::vector<double>& aNodalCoords);
/// @brief Given a BulkData @ aBulkData, the name of a sideset @ aSidesetName, and a list of blocks in the mesh @
/// aDesignDomainBlocks, return a vector of triangles that are in the sideset and whose normals point away from the
/// blocks. This function currently excludes blocks with "void" in the name.
[[nodiscard]] auto interface_triangles(const stk::mesh::BulkData& aBulkData,
                                       const std::string& aSidesetName,
                                       const PartReferenceVector& aDesignDomainBlocks)
    -> std::vector<SensitivityTriangle>;
/// @brief Transform the given flat vector with area sensitivities @ aFlatAreaSensitivityVector into meaningful data
/// structures.
[[nodiscard]] TriangleAreaSensitivity triangle_area_sensitivity(const std::vector<double>& aFlatAreaSensitivityVector);
/// @brief Transform the given flat vector with normal sensitivities @ aFlatNormalSensitivityVector into meaningful data
/// structures.
[[nodiscard]] TriangleNormalSensitivity triangle_normal_sensitivity(
    const std::vector<double>& aFlatNormalSensitivityVector);
/// @brief Given a triangle return a vector of its nodal coordinates.
[[nodiscard]] std::vector<double> convert_tri_to_coords(const SensitivityTriangle& aTriangle);

}  // end namespace detail

}  // namespace plato::third_party_integration::krino

#endif
