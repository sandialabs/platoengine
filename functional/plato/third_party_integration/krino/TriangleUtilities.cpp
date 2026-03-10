#include "plato/third_party_integration/krino/TriangleUtilities.hpp"

#include <Akri_OrientedSideNodes.hpp>
#include <Akri_TriangleWithSensitivities.hpp>
#include <ranges>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/Selector.hpp>

#include "plato/third_party_integration/krino/KrinoLevelSetPolicy.hpp"
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/MultiVectorView.hpp"
#include "plato/utilities/MultidimensionalRange.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::third_party_integration::krino
{

namespace
{
/// @brief Given a triangle return a vector of its nodal coordinates.
[[nodiscard]] auto convert_tri_to_coords(const SensitivityTriangle& aTriangle) -> std::vector<double>
{
    return {aTriangle.mNodes[0].second.x, aTriangle.mNodes[0].second.y, aTriangle.mNodes[0].second.z,
            aTriangle.mNodes[1].second.x, aTriangle.mNodes[1].second.y, aTriangle.mNodes[1].second.z,
            aTriangle.mNodes[2].second.x, aTriangle.mNodes[2].second.y, aTriangle.mNodes[2].second.z};
}

/// @brief Transform the given flat vector with area sensitivities @ aFlatAreaSensitivityVector into meaningful data
/// structures.
[[nodiscard]] auto triangle_area_sensitivity(const std::vector<double>& aFlatAreaSensitivityVector)
    -> TriangleAreaSensitivity
{
    // The area sensitivity flat vector coming from krino is ordered in the following way (3 values for each node):
    // Node 0:
    // aFlatAreaSensitivityVector[0] = dAreadp0X
    // aFlatAreaSensitivityVector[1] = dAreadp0Y
    // aFlatAreaSensitivityVector[2] = dAreadp0Z
    // Node 1:
    // aFlatAreaSensitivityVector[3] = dAreadp1X
    // aFlatAreaSensitivityVector[4] = dAreadp1Y
    // aFlatAreaSensitivityVector[5] = dAreadp1Z
    // ...

    constexpr size_t tNumberSpatialDimensions{3};
    constexpr size_t tNumberNodesPerTriangle{3};
    const auto tAreaSensitivityMultiVectorView =
        utilities::MultiVectorView(aFlatAreaSensitivityVector, tNumberSpatialDimensions);
    TriangleAreaSensitivity tSensitivities;
    constexpr auto tNodeRange = std::views::iota(0UL, tNumberNodesPerTriangle);
    std::transform(tNodeRange.begin(), tNodeRange.end(), tSensitivities.begin(),
                   [&tAreaSensitivityMultiVectorView](const auto tNodeIndex)
                   { return tAreaSensitivityMultiVectorView(utilities::VectorIndex{tNodeIndex}); });
    return tSensitivities;
}

/// @brief Transform the given flat vector with normal sensitivities @ aFlatNormalSensitivityVector into meaningful data
/// structures.
[[nodiscard]] auto triangle_normal_sensitivity(const std::vector<double>& aFlatNormalSensitivityVector)
    -> TriangleNormalSensitivity
{
    // The flat vector coming from krino is ordered in the following way (9 values for each node):
    // Node 0:
    // aFlatNormalSensitivityVector[0] = dNormalXdp0X
    // aFlatNormalSensitivityVector[1] = dNormalYdp0X
    // aFlatNormalSensitivityVector[2] = dNormalZdp0X
    // aFlatNormalSensitivityVector[3] = dNormalXdp0Y
    // aFlatNormalSensitivityVector[4] = dNormalYdp0Y
    // aFlatNormalSensitivityVector[5] = dNormalZdp0Y
    // aFlatNormalSensitivityVector[6] = dNormalXdp0Z
    // aFlatNormalSensitivityVector[7] = dNormalYdp0Z
    // aFlatNormalSensitivityVector[8] = dNormalZdp0Z
    // Node 1:
    // aFlatNormalSensitivityVector[9] = dNormalXdp1X
    // aFlatNormalSensitivityVector[10] = dNormalYdp1X
    // aFlatNormalSensitivityVector[11] = dNormalZdp1X
    // aFlatNormalSensitivityVector[12] = dNormalXdp1Y
    // ...

    constexpr size_t tNumberSpatialDimensions{3};
    constexpr size_t tNumberNodesPerTriangle{3};

    const auto tNormalSensitivityMultiVectorView =
        utilities::MultiVectorView(aFlatNormalSensitivityVector, tNumberSpatialDimensions);
    TriangleNormalSensitivity tSensitivities;

    for (const auto& [tNodeIndex, tComponentIndex] :
         utilities::MultidimensionalRange{tNumberNodesPerTriangle, tNumberSpatialDimensions})
    {
        tSensitivities[tNodeIndex][tComponentIndex] = tNormalSensitivityMultiVectorView(
            utilities::VectorIndex{tNodeIndex * tNumberNodesPerTriangle + tComponentIndex});
    }
    return tSensitivities;
}

}  // namespace

auto interface_sideset_name() -> std::string { return "surface__" + std::string{void_phase_name()}; }

TriangleAreaSensitivity d_area_d_tri_node(const SensitivityTriangle& aTriangle)
{
    // Get the change in triangle area and normal with changes in triangle nodal coordinates (3 triplets)
    const std::vector<double> tDAreaDNodalCoordinates =
        detail::d_area_d_nodal_coords_from_tri_coords(convert_tri_to_coords(aTriangle));
    return triangle_area_sensitivity(tDAreaDNodalCoordinates);
}

TriangleNormalSensitivity d_normal_d_tri_node(const SensitivityTriangle& aTriangle)
{
    // Get the change in triangle area and normal with changes in triangle nodal coordinates (3 triplets)
    const std::vector<double> tDNormalDCoords =
        detail::d_normal_d_nodal_coords_from_tri_coords(convert_tri_to_coords(aTriangle));
    return triangle_normal_sensitivity(tDNormalDCoords);
}

namespace detail
{

std::vector<double> d_area_d_nodal_coords_from_tri_coords(const std::vector<double>& aNodalCoords)
{
    constexpr auto tNumberSpatialDimensions{3};
    constexpr auto tNumberNodesPerTriangle{3};
    constexpr auto tNumSensitivities = tNumberSpatialDimensions * tNumberNodesPerTriangle;

    // Get the change in triangle area and normal with changes in triangle nodal coordinates (3 triplets)
    std::vector<double> tDAreaDNodalCoordinates(tNumSensitivities);
    ::krino::TriangleWithSens::area_and_optional_sensitivities(
        {aNodalCoords[0], aNodalCoords[1], aNodalCoords[2]}, {aNodalCoords[3], aNodalCoords[4], aNodalCoords[5]},
        {aNodalCoords[6], aNodalCoords[7], aNodalCoords[8]}, tDAreaDNodalCoordinates.data());
    return tDAreaDNodalCoordinates;
}

std::vector<double> d_normal_d_nodal_coords_from_tri_coords(const std::vector<double>& aNodalCoords)
{
    constexpr auto tNumberSpatialDimensions{3};
    constexpr auto tNumberNodesPerTriangle{3};
    constexpr auto tNumSensitivities = tNumberSpatialDimensions * tNumberNodesPerTriangle * tNumberSpatialDimensions;

    // Get the change in triangle area and normal with changes in triangle nodal coordinates (3 triplets)
    std::vector<double> tDNormalDNodalCoordinates(tNumSensitivities);
    ::krino::TriangleWithSens::normal_and_optional_sensitivities(
        {aNodalCoords[0], aNodalCoords[1], aNodalCoords[2]}, {aNodalCoords[3], aNodalCoords[4], aNodalCoords[5]},
        {aNodalCoords[6], aNodalCoords[7], aNodalCoords[8]}, tDNormalDNodalCoordinates.data());
    return tDNormalDNodalCoordinates;
}

std::vector<stk::mesh::Entity> get_owned_interface_sides(const stk::mesh::BulkData& aBulkData,
                                                         const stk::mesh::Selector& aInterfaceSelector)
{
    std::vector<stk::mesh::Entity> tInterfaceSides;

    for (auto* tBucket : aBulkData.get_buckets(aBulkData.mesh_meta_data().side_rank(),
                                               aBulkData.mesh_meta_data().locally_owned_part() & aInterfaceSelector))
    {
        tInterfaceSides.insert(tInterfaceSides.end(), tBucket->begin(), tBucket->end());
    }

    return tInterfaceSides;
}

auto interface_triangles(const stk::mesh::BulkData& aBulkData,
                         const std::string& aSidesetName,
                         const PartReferenceVector& aDesignDomainBlocks) -> std::vector<SensitivityTriangle>
{
    const auto tSidesetPart = aBulkData.mesh_meta_data().get_part(aSidesetName);
    if (!tSidesetPart)
    {
        return std::vector<SensitivityTriangle>();
    }

    const stk::mesh::Selector tTriSelector(*tSidesetPart);
    auto tParts = utilities::reserved_container<std::vector<const stk::mesh::Part*>>(aDesignDomainBlocks.size());
    utilities::transform_if(
        aDesignDomainBlocks, std::back_inserter(tParts),
        [](const auto& tCurrentBlock) { return &(tCurrentBlock.get()); }, [](const auto& tCurrentBlock)
        { return tCurrentBlock.get().name().find(void_phase_name()) == std::string::npos; });
    const stk::mesh::Selector tTetSelector{stk::mesh::selectUnion(tParts)};
    const std::vector<stk::mesh::Entity> tInterfaceSides = get_owned_interface_sides(aBulkData, tTriSelector);
    const stk::mesh::FieldBase* const tCoordsField = aBulkData.mesh_meta_data().coordinate_field();
    std::vector<SensitivityTriangle> tTriangles(tInterfaceSides.size());
    std::transform(tInterfaceSides.begin(), tInterfaceSides.end(), tTriangles.begin(),
                   [&aBulkData, &tTetSelector, tCoordsField](const auto& aInterfaceSide)
                   {
                       constexpr size_t tNumNodesPerTriangle{3};
                       const std::array<stk::mesh::Entity, 3> tSideNodes =
                           ::krino::get_oriented_triangle_side_nodes(aBulkData, tTetSelector, aInterfaceSide);
                       std::array<NodeIDCoordsPair, tNumNodesPerTriangle> tSensNodes;
                       for (const auto tNodeIndex : std::views::iota(0u, tNumNodesPerTriangle))
                       {
                           tSensNodes[tNodeIndex].first = aBulkData.identifier(tSideNodes[tNodeIndex]);
                           const double* tCoords =
                               static_cast<const double*>(stk::mesh::field_data(*tCoordsField, tSideNodes[tNodeIndex]));
                           tSensNodes[tNodeIndex].second.x = tCoords[0];
                           tSensNodes[tNodeIndex].second.y = tCoords[1];
                           tSensNodes[tNodeIndex].second.z = tCoords[2];
                       }
                       return SensitivityTriangle{tSensNodes[0], tSensNodes[1], tSensNodes[2]};
                   });

    return tTriangles;
}

}  // end namespace detail

}  // namespace plato::third_party_integration::krino
