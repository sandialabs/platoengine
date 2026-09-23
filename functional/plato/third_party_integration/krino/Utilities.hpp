#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_UTILITIES
#define PLATO_THIRDPARTYINTEGRATION_KRINO_UTILITIES

#include <mpi.h>

#include <Akri_ChildNodeStencil.hpp>         //ChildNodeStencil
#include <Akri_CreateInterfaceGeometry.hpp>  //LS_Field
#include <Akri_LevelSet.hpp>                 //LevelSet
#include <Akri_MeshFromFile.hpp>             //MeshInterface
#include <Akri_MeshHelpers.hpp>              //field_data
#include <boost/mpi/communicator.hpp>
#include <stk_mesh/base/Types.hpp>

namespace plato::third_party_integration::krino
{
/// @brief Initialization needed for krino to run correctly. This should only be called once in an executable.
void set_stk_communicator_for_krino(const boost::mpi::communicator& aCommunicator);

/// @brief Create a comm duplicate of the communicator that Krino has
[[nodiscard]] auto retrieve_mpi_communicator_from_krino() -> boost::mpi::communicator;

/// @brief Returns the setting for selecting node entities in krino meshes.
[[nodiscard]] auto sort_by_global_id() -> bool;

/// @brief Take a level set field vector @a aLevelSetFields and a node @a aNode and return the data stored in the mesh
/// @pre the environment for krino was initialized by calling 'set_stk_communicator_for_krino', and the level set fields
/// in the krino mesh were set up by constructing a background mesh.
template <typename LevelSetFieldVector>
[[nodiscard]] auto level_set_value(LevelSetFieldVector&& aLevelSetFields, const stk::mesh::Entity& aNode)
    -> decltype(auto);

/// @brief Simple accessor to grab the name of the level set field specified in a mesh.
[[nodiscard]] auto get_level_set_field_name() -> std::string;

/// @brief Returns the string used to represent the void phase in krino.
[[nodiscard]] auto void_phase_name() -> std::string_view;

namespace detail
{
/// @brief Gets IDs for the entities @a aEntities from entity keys in @a aBulkData.
[[nodiscard]] auto get_ids_from_entities(const std::vector<stk::mesh::Entity>& aEntities,
                                         const stk::mesh::BulkData& aBulkData) -> std::vector<stk::mesh::EntityId>;
}  // namespace detail

template <typename LevelSetFieldVector>
[[nodiscard]] auto level_set_value(LevelSetFieldVector&& aLevelSetFields,
                                   const stk::mesh::Entity& aNode) -> decltype(auto)
{
    assert(!aLevelSetFields.empty());
    return *::krino::field_data<double>(std::forward<LevelSetFieldVector>(aLevelSetFields).front().isovar, aNode);
}
}  // namespace plato::third_party_integration::krino

#endif
