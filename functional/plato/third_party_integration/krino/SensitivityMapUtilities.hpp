#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_SENSITIVITYMAP
#define PLATO_THIRDPARTYINTEGRATION_KRINO_SENSITIVITYMAP

#include <Akri_ChildNodeStencil.hpp>         //ChildNodeStencil
#include <Akri_CreateInterfaceGeometry.hpp>  //LS_Field
#include <Akri_LevelSet.hpp>
#include <Akri_LevelSetPolicy.hpp>
#include <Akri_LevelSetShapeSensitivities.hpp>
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

/// @brief Return sensitivities of nodal coordinates with respect to the nodal level set field as computed by krino.
///
/// @pre The environment for krino must be initialized and set up as follows:
/// @code{.cpp}
/// initialize_environment_for_krino(...);
/// read_and_setup_for_decomposition(...);
/// cut_mesh(...);
/// @endcode
[[nodiscard]] auto get_krino_sensitivities(const stk::mesh::BulkData& aBulkData,
                                           const std::vector<::krino::LS_Field>& aLevelSetFields)
    -> std::vector<::krino::LevelSetShapeSensitivity>;

/// @brief Return a vector of derivatives of each nodal coordinate with respect to the nodal level set field. The
/// coordinate sensitivities are stored in a Vector3, with z coordinates set to 0 if @a aSpatialDimension is 2.
[[nodiscard]] auto coordinate_level_set_sensitivity(const ::krino::LevelSetShapeSensitivity& aLevelSetSensitivity,
                                                    const unsigned int aSpatialDimension)
    -> std::vector<common::Vector3>;

/// @brief Compute the repeated occurrence of cut mesh node ids in parallel using sensitivity map @a aSensitivityMap.
/// @note Sensitivity maps are computing on their local rank and only contain data from that rank.
/// @post Return a mpi consistent unorderd map that contains only the duplicated cut mesh ids.
[[nodiscard]] auto cut_mesh_node_id_multiplicity(const SensitivityMap& aSensitivityMap)
    -> std::unordered_map<CutMeshSurfaceNodeId, unsigned int>;

namespace detail
{

/// @brief Take a sorted vector of cut mesh ids @a aGatheredSortedCutMeshNodeIDs and determine a histogram of ids that
/// are repeated more than once.
/// @pre @a aGatheredSortedCutMeshNodeIDs is the complete set across ranks and is sorted.
[[nodiscard]] auto compute_histogram(const std::vector<stk::mesh::EntityId>& aGatheredSortedCutMeshNodeIDs)
    -> std::unordered_map<stk::mesh::EntityId, unsigned int>;

}  // namespace detail

}  // namespace plato::third_party_integration::krino

#endif
