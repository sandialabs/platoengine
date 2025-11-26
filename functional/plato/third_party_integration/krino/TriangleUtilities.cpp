#include "plato/third_party_integration/krino/TriangleUtilities.hpp"

#include <Akri_OrientedSideNodes.hpp>
#include <Akri_TriangleWithSensitivities.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/Selector.hpp>

#include "plato/utilities/MultiVectorView.hpp"

namespace plato::third_party_integration::krino
{

TriangleAreaSensitivity get_d_area_d_tri_node(const SensitivityTriangle& aTriangle)
{
    // Get the change in triangle area and normal with changes in triangle nodal coordinates (3 triplets)
    const std::vector<double> tDAreaDNodalCoordinates = detail::get_d_area_d_nodal_coords_from_tri_coords(
        {aTriangle.mNodes[0].second.x, aTriangle.mNodes[0].second.y, aTriangle.mNodes[0].second.z,
         aTriangle.mNodes[1].second.x, aTriangle.mNodes[1].second.y, aTriangle.mNodes[1].second.z,
         aTriangle.mNodes[2].second.x, aTriangle.mNodes[2].second.y, aTriangle.mNodes[2].second.z});
    return detail::convert_area_sensitivties_from_flat_vector_to_plato_data_structure(tDAreaDNodalCoordinates);
}

TriangleNormalSensitivity get_d_normal_d_tri_node(const SensitivityTriangle& aTriangle)
{
    // Get the change in triangle area and normal with changes in triangle nodal coordinates (3 triplets)
    const std::vector<double> tDNormalDCoords = detail::get_d_normal_d_nodal_coords_from_tri_coords(
        {aTriangle.mNodes[0].second.x, aTriangle.mNodes[0].second.y, aTriangle.mNodes[0].second.z,
         aTriangle.mNodes[1].second.x, aTriangle.mNodes[1].second.y, aTriangle.mNodes[1].second.z,
         aTriangle.mNodes[2].second.x, aTriangle.mNodes[2].second.y, aTriangle.mNodes[2].second.z});
    return detail::convert_normal_sensitivities_from_flat_vector_to_plato_data_structure(tDNormalDCoords);
}

namespace detail
{

std::vector<double> get_d_area_d_nodal_coords_from_tri_coords(const std::vector<double>& aNodalCoords)
{
    constexpr auto tNumberSpatialDimensions{3};
    constexpr auto tNumberNodesPerTriangle{3};
    constexpr auto tNumSensitivities = tNumberSpatialDimensions * tNumberNodesPerTriangle;

    // Get the change in triangle area and normal with changes in triangle nodal coordinates (3 triplets)
    std::vector<double> tDAreaDNodalCoordinates(tNumSensitivities);
    const stk::math::Vector3d tPoint0{aNodalCoords[0], aNodalCoords[1], aNodalCoords[2]};
    const stk::math::Vector3d tPoint1{aNodalCoords[3], aNodalCoords[4], aNodalCoords[5]};
    const stk::math::Vector3d tPoint2{aNodalCoords[6], aNodalCoords[7], aNodalCoords[8]};
    ::krino::TriangleWithSens::area_and_optional_sensitivities(tPoint0, tPoint1, tPoint2,
                                                               tDAreaDNodalCoordinates.data());
    return tDAreaDNodalCoordinates;
}

std::vector<double> get_d_normal_d_nodal_coords_from_tri_coords(const std::vector<double>& aNodalCoords)
{
    constexpr auto tNumberSpatialDimensions{3};
    constexpr auto tNumberNodesPerTriangle{3};
    constexpr auto tNumSensitivities = tNumberSpatialDimensions * tNumberNodesPerTriangle * tNumberSpatialDimensions;

    // Get the change in triangle area and normal with changes in triangle nodal coordinates (3 triplets)
    std::vector<double> tDNormalDNodalCoordinates(tNumSensitivities);
    const stk::math::Vector3d tPoint0{aNodalCoords[0], aNodalCoords[1], aNodalCoords[2]};
    const stk::math::Vector3d tPoint1{aNodalCoords[3], aNodalCoords[4], aNodalCoords[5]};
    const stk::math::Vector3d tPoint2{aNodalCoords[6], aNodalCoords[7], aNodalCoords[8]};
    ::krino::TriangleWithSens::normal_and_optional_sensitivities(tPoint0, tPoint1, tPoint2,
                                                                 tDNormalDNodalCoordinates.data());
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

auto get_interface_triangles(const stk::mesh::BulkData& aBulkData,
                             const std::string& aSidesetName,
                             const PartReferenceVector& aDesignDomainBlocks) -> std::vector<SensitivityTriangle>
{
    const auto tSidesetPart = aBulkData.mesh_meta_data().get_part(aSidesetName);
    if (!tSidesetPart)
    {
        return std::vector<SensitivityTriangle>();
    }

    const stk::mesh::Selector tTriSelector(*tSidesetPart);
    std::vector<const stk::mesh::Part*> tParts;
    for (const auto& tCurBlock : aDesignDomainBlocks)
    {
        if (tCurBlock.get().name().find("void") == std::string::npos)
        {
            tParts.push_back(&(tCurBlock.get()));
        }
    }
    const stk::mesh::Selector tTetSelector{stk::mesh::selectUnion(tParts)};
    const std::vector<stk::mesh::Entity> tInterfaceSides = get_owned_interface_sides(aBulkData, tTriSelector);
    const stk::mesh::FieldBase* const tCoordsField = aBulkData.mesh_meta_data().coordinate_field();
    std::vector<SensitivityTriangle> tTriangles(tInterfaceSides.size());
    size_t tNumTris = 0;
    for (const auto& tInterfaceSide : tInterfaceSides)
    {
        const std::array<stk::mesh::Entity, 3> tSideNodes =
            ::krino::get_oriented_triangle_side_nodes(aBulkData, tTetSelector, tInterfaceSide);
        std::array<NodeIDCoordsPair, 3> tSensNodes;
        for (size_t i = 0; i < 3; ++i)
        {
            tSensNodes[i].first = aBulkData.identifier(tSideNodes[i]);
            const double* tCoords = static_cast<const double*>(stk::mesh::field_data(*tCoordsField, tSideNodes[i]));
            tSensNodes[i].second.x = tCoords[0];
            tSensNodes[i].second.y = tCoords[1];
            tSensNodes[i].second.z = tCoords[2];
        }
        tTriangles[tNumTris++] = SensitivityTriangle{tSensNodes[0], tSensNodes[1], tSensNodes[2]};
    }
    return tTriangles;
}

TriangleAreaSensitivity convert_area_sensitivties_from_flat_vector_to_plato_data_structure(
    const std::vector<double>& aFlatAreaSensitivityVector)
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
    for (size_t tNodeIndex = 0; tNodeIndex < tNumberNodesPerTriangle; tNodeIndex++)
    {
        DAreaDNode dAreadNode;
        dAreadNode.x =
            tAreaSensitivityMultiVectorView(utilities::VectorIndex{tNodeIndex}, utilities::ComponentIndex{0});
        dAreadNode.y =
            tAreaSensitivityMultiVectorView(utilities::VectorIndex{tNodeIndex}, utilities::ComponentIndex{1});
        dAreadNode.z =
            tAreaSensitivityMultiVectorView(utilities::VectorIndex{tNodeIndex}, utilities::ComponentIndex{2});
        tSensitivities[tNodeIndex] = dAreadNode;
    }
    return tSensitivities;
}

TriangleNormalSensitivity convert_normal_sensitivities_from_flat_vector_to_plato_data_structure(
    const std::vector<double>& aFlatNormalSensitivityVector)
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
    constexpr size_t tNumNodeSensitivities = tNumberSpatialDimensions * tNumberSpatialDimensions;

    const auto tNormalSensitivityMultiVectorView =
        utilities::MultiVectorView(aFlatNormalSensitivityVector, tNumNodeSensitivities);
    TriangleNormalSensitivity tSensitivities;
    for (size_t tNodeIndex = 0; tNodeIndex < tNumberNodesPerTriangle; tNodeIndex++)
    {
        DNormalDNodeCoordinate dNormaldNodeX;
        dNormaldNodeX.x =
            tNormalSensitivityMultiVectorView(utilities::VectorIndex{tNodeIndex}, utilities::ComponentIndex{0});
        dNormaldNodeX.y =
            tNormalSensitivityMultiVectorView(utilities::VectorIndex{tNodeIndex}, utilities::ComponentIndex{1});
        dNormaldNodeX.z =
            tNormalSensitivityMultiVectorView(utilities::VectorIndex{tNodeIndex}, utilities::ComponentIndex{2});
        tSensitivities[tNodeIndex][kDNormalDNodeXCoord] = dNormaldNodeX;
        DNormalDNodeCoordinate dNormaldNodeY;
        dNormaldNodeY.x =
            tNormalSensitivityMultiVectorView(utilities::VectorIndex{tNodeIndex}, utilities::ComponentIndex{3});
        dNormaldNodeY.y =
            tNormalSensitivityMultiVectorView(utilities::VectorIndex{tNodeIndex}, utilities::ComponentIndex{4});
        dNormaldNodeY.z =
            tNormalSensitivityMultiVectorView(utilities::VectorIndex{tNodeIndex}, utilities::ComponentIndex{5});
        tSensitivities[tNodeIndex][kDNormalDNodeYCoord] = dNormaldNodeY;
        DNormalDNodeCoordinate dNormaldNodeZ;
        dNormaldNodeZ.x =
            tNormalSensitivityMultiVectorView(utilities::VectorIndex{tNodeIndex}, utilities::ComponentIndex{6});
        dNormaldNodeZ.y =
            tNormalSensitivityMultiVectorView(utilities::VectorIndex{tNodeIndex}, utilities::ComponentIndex{7});
        dNormaldNodeZ.z =
            tNormalSensitivityMultiVectorView(utilities::VectorIndex{tNodeIndex}, utilities::ComponentIndex{8});
        tSensitivities[tNodeIndex][kDNormalDNodeZCoord] = dNormaldNodeZ;
    }
    return tSensitivities;
}

}  // end namespace detail

}  // namespace plato::third_party_integration::krino
