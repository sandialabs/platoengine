#include "plato/third_party_integration/krino/SensitivityMapUtilities.hpp"

#include <Akri_CDFEM_Support.hpp>     //CDFEM_Support
#include <Akri_ChildNodeStencil.hpp>  //ChildNodeStencil
#include <Akri_LevelSet.hpp>          //LevelSet
#include <Akri_LevelSetPolicy.hpp>    //LSPerInterfacePolicy
#include <stk_mesh/base/Entity.hpp>
#include <stk_mesh/base/Types.hpp>

#include "plato/utilities/NamedType.hpp"

namespace plato::third_party_integration::krino
{
namespace
{
[[maybe_unused]] constexpr auto kNumberOfParents = std::size_t{2};
constexpr auto kFirstParentIndex = 0U;
constexpr auto kSecondParentIndex = 1U;

[[nodiscard]] auto read_scalar_field(const ::krino::FieldRef aField, const stk::mesh::Entity aID) -> double
{
    return *::krino::field_data<double>(aField, aID);
}

[[nodiscard]] auto read_vector_field(const ::krino::FieldRef aField,
                                     const stk::mesh::Entity aID,
                                     const unsigned int aSpatialDimension) -> common::Vector3
{
    const auto* tVector = ::krino::field_data<double>(aField, aID);
    const auto tZComponent = aSpatialDimension == 3U ? tVector[2] : 0.0;
    return common::Vector3{tVector[0], tVector[1], tZComponent};
}

[[nodiscard]] auto read_coordinate_and_level_set_field(const CoordinateFieldReference aCoordsField,
                                                       const LevelSetFieldReference aLevelSetField,
                                                       const stk::mesh::Entity aID,
                                                       const unsigned int aSpatialDimension)
    -> std::pair<common::Vector3, double>
{
    return std::make_pair(read_vector_field(aCoordsField.mValue, aID, aSpatialDimension),
                          read_scalar_field(aLevelSetField.mValue, aID));
}

}  // namespace

/// Given two points $\vec{x_1}$ and $\vec{x_2}$ and their associated level set values $L_1$ and $L_2$.
/// Assuming there is a point between them that corresponds to $L = 0$, we can form the two point form of the linear
/// relation:
/// $(L - L_1) = \frac{L_2 - L_1}{\vec{x_2}-\vec{x_1}}\left(\vec{x}-\vec{x_2})$
/// Setting $L=0$, and solving for $\vec{x}$ gives the nodal coordinate that is at the 0 point for the level set field.
/// This is:
/// $\vec{x} = \frac{- L_1}{L_2-L_1}\left(\vec{x_2}-\vec{x_1}\right) + \vec{x_1}$
/// The partial derivative of this w.r.t. $L_1$ and $L_2$ are:
/// $\frac{\partial x}{\partial L_1} = (x_2 - x_1) \frac{-L_2}{(L_2 - L_1)^2}$
/// $\frac{\partial x}{\partial L_2} = (x_2 - x_1) \frac{L_1}{(L_2 - L_1)^2}$
auto level_set_coordinate_sensitivity(const CoordinateFieldReference aCoordinateField,
                                      const LevelSetFieldReference aLevelSetField,
                                      const std::vector<stk::mesh::Entity>& aParentNodes,
                                      const unsigned int aSpatialDimension) -> std::vector<common::Vector3>
{
    assert(kNumberOfParents == aParentNodes.size());

    const auto [tFirstCoordinate, tFirstLevelSetValue] = read_coordinate_and_level_set_field(
        aCoordinateField, aLevelSetField, aParentNodes[kFirstParentIndex], aSpatialDimension);
    const auto [tSecondCoordinate, tSecondLevelSetValue] = read_coordinate_and_level_set_field(
        aCoordinateField, aLevelSetField, aParentNodes[kSecondParentIndex], aSpatialDimension);
    const auto tDisplacementVector = tSecondCoordinate - tFirstCoordinate;
    const auto tSquaredLevelSetDifference =
        (tFirstLevelSetValue - tSecondLevelSetValue) * (tFirstLevelSetValue - tSecondLevelSetValue);

    return std::vector<common::Vector3>{tDisplacementVector * (-tSecondLevelSetValue / tSquaredLevelSetDifference),
                                        tDisplacementVector * (tFirstLevelSetValue / tSquaredLevelSetDifference)};
}

auto get_child_node_stencils(const stk::mesh::BulkData& aBulkData) -> std::vector<::krino::ChildNodeStencil>
{
    const ::krino::CDFEM_Support& cdfemSupport = ::krino::CDFEM_Support::get(aBulkData.mesh_meta_data());
    std::vector<::krino::ChildNodeStencil> tChildNodeStencils;
    ::krino::fill_child_node_stencils(aBulkData, cdfemSupport.get_child_node_part(),
                                      cdfemSupport.get_parent_node_ids_field(),
                                      cdfemSupport.get_parent_node_weights_field(), tChildNodeStencils);
    return tChildNodeStencils;
}

auto parent_node_ids_from_parent_nodes(const stk::mesh::BulkData& aBulkData,
                                       const std::vector<ParentNode>& aParentNodes) -> std::vector<BackgroundMeshNodeId>
{
    assert(aParentNodes.size() == kNumberOfParents);

    const auto tParentIDFront = aBulkData.identifier(aParentNodes.front());
    const auto tParentIDBack = aBulkData.identifier(aParentNodes.back());

    return std::vector{tParentIDFront, tParentIDBack};
    // return tParentIDFront < tParentIDBack ? std::vector{tParentIDFront, tParentIDBack}
    //                                      : std::vector{tParentIDBack, tParentIDFront};
}
namespace detail
{

auto merge_sensitivity_maps(AppendMap aAppendMap, const OtherMap& aOtherMap) -> SensitivityMap
{
    auto tAppendMap = std::move(aAppendMap).mValue;
    for (const auto& [tCutMeshId, tLevelSetJacobianColumn] : aOtherMap.mValue)
    {
        if (const auto tIterator = tAppendMap.find(tCutMeshId); tIterator != tAppendMap.end())
        {
            auto& tAppendLevelSetJacobian = tIterator->second;
            AppendLevelSetJacobianColumn tAppend{tAppendLevelSetJacobian};
            tAppendMap[tCutMeshId] =
                merge_level_set_jacobian_columns(tAppend, OtherLevelSetJacobianColumn{tLevelSetJacobianColumn});
        }
        else
        {
            tAppendMap[tCutMeshId] = tLevelSetJacobianColumn;
        }
    }
    return tAppendMap;
}

auto merge_level_set_jacobian_columns(AppendLevelSetJacobianColumn aAppendLevelSetJacobianColumn,
                                      const OtherLevelSetJacobianColumn& aOtherLevelSetJacobianColumn)
    -> LevelSetJacobianColumn
{
    if (aAppendLevelSetJacobianColumn.mValue.mBackgroundMeshNodeIDs.size() == 2)
    {
        return aAppendLevelSetJacobianColumn.mValue;
    }

    if (aOtherLevelSetJacobianColumn.mValue.mBackgroundMeshNodeIDs.size() == 2)
    {
        return aOtherLevelSetJacobianColumn.mValue;
    }

    auto tLevelSetJacobianColumn = std::move(aAppendLevelSetJacobianColumn).mValue;
    if (tLevelSetJacobianColumn.mBackgroundMeshNodeIDs.size() == 0 ||
        tLevelSetJacobianColumn.mBackgroundMeshNodeIDs.front() !=
            aOtherLevelSetJacobianColumn.mValue.mBackgroundMeshNodeIDs.front())
    {
        tLevelSetJacobianColumn.mBackgroundMeshNodeIDs.push_back(
            aOtherLevelSetJacobianColumn.mValue.mBackgroundMeshNodeIDs.front());
        tLevelSetJacobianColumn.mNodalSensitivities.push_back(
            aOtherLevelSetJacobianColumn.mValue.mNodalSensitivities.front());
        tLevelSetJacobianColumn.mDesignDomainLocalIndex.push_back(
            aOtherLevelSetJacobianColumn.mValue.mDesignDomainLocalIndex.front());
    }

    if (tLevelSetJacobianColumn.mBackgroundMeshNodeIDs.size() > 2)
    {
        std::cout << "larger than I thought..." << std::endl;
    }
    return tLevelSetJacobianColumn;
}

}  // namespace detail
}  // namespace plato::third_party_integration::krino
