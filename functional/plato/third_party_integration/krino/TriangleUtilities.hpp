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
[[nodiscard]] auto d_normal_d_tri_node(const SensitivityTriangle& aTriangle) -> TriangleNormalSensitivity;
/// @brief Given a triangle @ aTriangle calculate the area nodal sensitivities.
[[nodiscard]] auto d_area_d_tri_node(const SensitivityTriangle& aTriangle) -> TriangleAreaSensitivity;

namespace detail
{

/// @brief Given the nodal coordinates of a triangle @ aNodalCoords calculate the area sensitivities to changes in nodal
/// coordinates.
[[nodiscard]] auto d_area_d_nodal_coords_from_tri_coords(const std::vector<double>& aNodalCoords)
    -> std::vector<double>;

/// @brief Given the nodal coordinates of a triangle @ aNodalCoords calculate the normal sensitivities to changes in
/// nodal coordinates.
[[nodiscard]] auto d_normal_d_nodal_coords_from_tri_coords(const std::vector<double>& aNodalCoords)
    -> std::vector<double>;

/// @brief Given a BulkData @ aBulkData, the name of a sideset @ aSidesetName, and a list of blocks in the mesh @
/// aDesignDomainBlocks, return a vector of triangles that are in the sideset and whose normals point away from the
/// blocks. This function currently excludes blocks with "void" in the name.
[[nodiscard]] auto interface_triangles(const stk::mesh::BulkData& aBulkData,
                                       const std::string& aSidesetName,
                                       const PartReferenceVector& aDesignDomainBlocks)
    -> std::vector<SensitivityTriangle>;
/// @brief Transform the given flat vector with area sensitivities @ aFlatAreaSensitivityVector into meaningful data
/// structures.
[[nodiscard]] auto triangle_area_sensitivity(const std::vector<double>& aFlatAreaSensitivityVector)
    -> TriangleAreaSensitivity;
/// @brief Transform the given flat vector with normal sensitivities @ aFlatNormalSensitivityVector into meaningful data
/// structures.
[[nodiscard]] auto triangle_normal_sensitivity(const std::vector<double>& aFlatNormalSensitivityVector)
    -> TriangleNormalSensitivity;
/// @brief Given a triangle return a vector of its nodal coordinates.
[[nodiscard]] auto convert_tri_to_coords(const SensitivityTriangle& aTriangle) -> std::vector<double>;

}  // end namespace detail

}  // namespace plato::third_party_integration::krino

#endif
