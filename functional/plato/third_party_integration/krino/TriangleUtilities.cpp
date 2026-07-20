#include "plato/third_party_integration/krino/TriangleUtilities.hpp"

#include <Akri_OrientedSideNodes.hpp>
#include <Akri_TriangleWithSensitivities.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <span>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Entity.hpp>
#include <stk_mesh/base/FieldBase.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/Selector.hpp>
#include <string>
#include <vector>

#include "plato/third_party_integration/krino/SensitivityTriangle.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/MultiVectorView.hpp"
#include "plato/utilities/MultidimensionalRange.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::third_party_integration::krino
{

namespace
{
template <std::size_t kFieldSize>
[[nodiscard]] auto to_span(void* const aStkField)
{
    return std::span<double, kFieldSize>(static_cast<double*>(aStkField), kFieldSize);
}

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
                   [&tAreaSensitivityMultiVectorView](const auto aNodeIndex)
                   { return tAreaSensitivityMultiVectorView(utilities::VectorIndex{aNodeIndex}); });
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
        tSensitivities.at(tNodeIndex).at(tComponentIndex) = tNormalSensitivityMultiVectorView(
            utilities::VectorIndex{tNodeIndex * tNumberNodesPerTriangle + tComponentIndex});
    }
    return tSensitivities;
}

template <std::size_t kVectorIndex>
[[nodiscard]] auto to_stk_vector_at_index(const std::vector<double>& aNodalCoordinates)
{
    constexpr auto tNumberOfNodesPerTriangle = std::size_t{3U};
    return stk::math::Vector3d{aNodalCoordinates[kVectorIndex * tNumberOfNodesPerTriangle + 0U],
                               aNodalCoordinates[kVectorIndex * tNumberOfNodesPerTriangle + 1U],
                               aNodalCoordinates[kVectorIndex * tNumberOfNodesPerTriangle + 2U]};
}

}  // namespace

auto interface_sideset_name() -> std::string { return "surface__" + std::string{void_phase_name()}; }

auto d_area_d_tri_node(const SensitivityTriangle& aTriangle) -> TriangleAreaSensitivity
{
    // Get the change in triangle area and normal with changes in triangle nodal coordinates (3 triplets)
    const std::vector<double> tDAreaDNodalCoordinates =
        detail::d_area_d_nodal_coords_from_tri_coords(convert_tri_to_coords(aTriangle));
    return triangle_area_sensitivity(tDAreaDNodalCoordinates);
}

auto d_normal_d_tri_node(const SensitivityTriangle& aTriangle) -> TriangleNormalSensitivity
{
    // Get the change in triangle area and normal with changes in triangle nodal coordinates (3 triplets)
    const std::vector<double> tDNormalDCoords =
        detail::d_normal_d_nodal_coords_from_tri_coords(convert_tri_to_coords(aTriangle));
    return triangle_normal_sensitivity(tDNormalDCoords);
}

namespace detail
{

auto d_area_d_nodal_coords_from_tri_coords(const std::vector<double>& aNodalCoords) -> std::vector<double>
{
    constexpr auto tNumberSpatialDimensions{3};
    constexpr auto tNumberNodesPerTriangle{3};
    constexpr auto tNumSensitivities = tNumberSpatialDimensions * tNumberNodesPerTriangle;

    // Get the change in triangle area and normal with changes in triangle nodal coordinates (3 triplets)
    std::vector<double> tDAreaDNodalCoordinates(tNumSensitivities);
    ::krino::TriangleWithSens::area_and_optional_sensitivities(
        to_stk_vector_at_index<0U>(aNodalCoords), to_stk_vector_at_index<1U>(aNodalCoords),
        to_stk_vector_at_index<2U>(aNodalCoords), tDAreaDNodalCoordinates.data());
    return tDAreaDNodalCoordinates;
}

auto d_normal_d_nodal_coords_from_tri_coords(const std::vector<double>& aNodalCoords) -> std::vector<double>
{
    constexpr auto tNumberSpatialDimensions{3};
    constexpr auto tNumberNodesPerTriangle{3};
    constexpr auto tNumSensitivities = tNumberSpatialDimensions * tNumberNodesPerTriangle * tNumberSpatialDimensions;

    // Get the change in triangle area and normal with changes in triangle nodal coordinates (3 triplets)
    std::vector<double> tDNormalDNodalCoordinates(tNumSensitivities);
    ::krino::TriangleWithSens::normal_and_optional_sensitivities(
        to_stk_vector_at_index<0U>(aNodalCoords), to_stk_vector_at_index<1U>(aNodalCoords),
        to_stk_vector_at_index<2U>(aNodalCoords), tDNormalDNodalCoordinates.data());
    return tDNormalDNodalCoordinates;
}

auto get_owned_interface_sides(const stk::mesh::BulkData& aBulkData,
                               const stk::mesh::Selector& aInterfaceSelector) -> std::vector<stk::mesh::Entity>
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
    auto* const tSidesetPart = aBulkData.mesh_meta_data().get_part(aSidesetName);
    if (tSidesetPart == nullptr)
    {
        return {};
    }

    const stk::mesh::Selector tTriSelector(*tSidesetPart);
    auto tParts = utilities::reserved_container<std::vector<const stk::mesh::Part*>>(aDesignDomainBlocks.size());
    utilities::transform_if(
        aDesignDomainBlocks, std::back_inserter(tParts),
        [](const auto& aCurrentBlock) { return &(aCurrentBlock.get()); }, [](const auto& aCurrentBlock)
        { return aCurrentBlock.get().name().find(void_phase_name()) == std::string::npos; });
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
                       for (const auto tNodeIndex : std::views::iota(0U, tNumNodesPerTriangle))
                       {
                           tSensNodes.at(tNodeIndex).first = aBulkData.identifier(tSideNodes.at(tNodeIndex));
                           constexpr auto tCoordinateFieldSize = std::size_t{3U};
                           const auto tCoordinates = to_span<tCoordinateFieldSize>(
                               stk::mesh::field_data(*tCoordsField, tSideNodes.at(tNodeIndex)));
                           tSensNodes.at(tNodeIndex).second.x = tCoordinates[0];
                           tSensNodes.at(tNodeIndex).second.y = tCoordinates[1];
                           tSensNodes.at(tNodeIndex).second.z = tCoordinates[2];
                       }
                       return SensitivityTriangle{tSensNodes[0], tSensNodes[1], tSensNodes[2]};
                   });

    return tTriangles;
}

}  // end namespace detail

}  // namespace plato::third_party_integration::krino
