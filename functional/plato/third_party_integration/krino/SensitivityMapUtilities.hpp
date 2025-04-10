#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_SENSITIVITYMAP
#define PLATO_THIRDPARTYINTEGRATION_KRINO_SENSITIVITYMAP

#include <Akri_ChildNodeStencil.hpp>         //ChildNodeStencil
#include <Akri_CreateInterfaceGeometry.hpp>  //LS_Field
#include <Akri_LevelSet.hpp>
#include <Akri_LevelSetPolicy.hpp>
#include <stk_mesh/base/Types.hpp>

#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::third_party_integration::krino
{

using CutMeshSurfaceNodeId = stk::mesh::EntityId;
using BackgroundMeshNodeId = stk::mesh::EntityId;
using DesignDomainLocalId = std::size_t;
using ParentNode = stk::mesh::Entity;
/// @brief Data that describes a column of the Jacobian of the level-set mapping, i.e. the interface coordinate
/// sensitivities.
///
/// The level-set mapping maps a scalar level set field on a background mesh (and has dimensions equal to the number of
/// nodes on the background mesh) to cut mesh nodal coordinates on the cut mesh interface.
struct LevelSetJacobianColumn
{
    std::vector<BackgroundMeshNodeId> mBackgroundMeshNodeIDs;
    std::vector<common::Vector3> mNodalSensitivities;
    std::vector<DesignDomainLocalId> mDesignDomainLocalIndex;
};

using SensitivityMap = std::unordered_map<CutMeshSurfaceNodeId, LevelSetJacobianColumn>;

using CoordinateFieldReference = utilities::NamedType<::krino::FieldRef, struct CoordinateFieldReferenceTag>;
using LevelSetFieldReference = utilities::NamedType<::krino::FieldRef, struct LevelSetFieldReferenceTag>;

/// @brief Take a krino mesh's bulk data @a aBulkData, and retrieve a vector of the child node stencils.
///
/// Child node stencils provide information about who the parents are of that child node.
/// @pre The environment for krino must be initialized and set up as follows:
/// @code{.cpp}
/// initialize_environment_for_krino(...);
/// read_and_setup_for_decomposition(...);
/// setup_level_set_field_values(...);
/// cut_mesh(...);
/// @endcode
[[nodiscard]] auto get_child_node_stencils(const stk::mesh::BulkData& aBulkData)
    -> std::vector<::krino::ChildNodeStencil>;

/// @brief Take a krino mesh's bulk data @a aBulkData, and a vector of parent nodes @a aParentNodes and extract the stk
/// mesh ids for the parents
///
/// @pre The environment for krino must be initialized and set up as follows:
/// @code{.cpp}
/// initialize_environment_for_krino(...);
/// read_and_setup_for_decomposition(...);
/// setup_level_set_field_values(...);
/// cut_mesh(...);
/// @endcode
[[nodiscard]] auto parent_node_ids_from_parent_nodes(const stk::mesh::BulkData& aBulkData,
                                                     const std::vector<ParentNode>& aParentNodes)
    -> std::vector<BackgroundMeshNodeId>;

/// @brief Compute the coordinate level set by taking field data references for the coordinates @a aCoordinateField and
/// the level set field @a aLevelSetField, and using the parent nodes @a aParentNodes along with   krino mesh's bulk
/// data @a aBulkData, and a vector of parent nodes @a aParentNodes. Perform the calculation using the spatial dimension
/// specified in @a aSpatialDimension
///
/// @pre The environment for krino must be initialized and set up as follows:
/// @code{.cpp}
/// initialize_environment_for_krino(...);
/// read_and_setup_for_decomposition(...);
/// setup_level_set_field_values(...);
/// cut_mesh(...);
/// @endcode
[[nodiscard]] auto level_set_coordinate_sensitivity(const CoordinateFieldReference aCoordinateField,
                                                    const LevelSetFieldReference aLevelSetField,
                                                    const std::vector<stk::mesh::Entity>& aParentNodes,
                                                    const unsigned int aSpatialDimension)
    -> std::vector<common::Vector3>;

namespace detail
{
using AppendMap = utilities::NamedType<SensitivityMap, struct AppendMapTag>;
using OtherMap = utilities::NamedType<SensitivityMap, struct OtherMapTag>;
[[nodiscard]] auto merge_sensitivity_maps(AppendMap aAppendMap, const OtherMap& aOtherMap) -> SensitivityMap;

using AppendLevelSetJacobianColumn =
    utilities::NamedType<LevelSetJacobianColumn, struct AppendLevelSetJacobianColumnTag>;
using OtherLevelSetJacobianColumn = utilities::NamedType<LevelSetJacobianColumn, struct OtherLevelSetJacobianColumnTag>;
[[nodiscard]] auto merge_level_set_jacobian_columns(AppendLevelSetJacobianColumn aAppendLevelSetJacobianColumn,
                                                    const OtherLevelSetJacobianColumn& aOtherLevelSetJacobianColumn)
    -> LevelSetJacobianColumn;

}  // namespace detail

}  // namespace plato::third_party_integration::krino

#endif
