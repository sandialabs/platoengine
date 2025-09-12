#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_UTILITIES
#define PLATO_THIRDPARTYINTEGRATION_KRINO_UTILITIES

#include <mpi.h>

#include <Akri_ChildNodeStencil.hpp>         //ChildNodeStencil
#include <Akri_CreateInterfaceGeometry.hpp>  //LS_Field
#include <Akri_LevelSet.hpp>                 //LevelSet
#include <Akri_MeshFromFile.hpp>             //MeshInterface
#include <Akri_MeshHelpers.hpp>              //field_data
#include <boost/mpi/communicator.hpp>
#include <cstdint>
#include <filesystem>
#include <stk_mesh/base/Types.hpp>

#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"
#include "plato/third_party_integration/krino/SnappingParameters.hpp"

namespace plato::third_party_integration::krino
{

/// @brief Describes whether or not to include a void phase block in the generated cut mesh.
enum struct VoidPhase : std::uint8_t
{
    kIncludeInMesh,
    kExcludeFromMesh
};

/// @brief Initialization needed for krino to run correctly. This should only be called once in an executable.
void initialize_environment_for_krino(const std::filesystem::path& aLogFile, const MPI_Comm& aComm);

/// @brief Create a comm duplicate of the communicator that Krino has
[[nodiscard]] auto retrieve_mpi_communicator_from_krino() -> boost::mpi::communicator;

/// @brief Creates a Krino mesh from a mesh on disk at @a aFilename that excludes blocks with names given in @a
/// aExcludedBlocks. Typically, @a aExcludedBlocks will be the fixed (non-design) blocks.
/// @pre The environment for krino was initialized.
/// @pre @a aFileName is a valid exodus mesh file on disk.
[[nodiscard]] auto read_and_setup_for_decomposition(const std::filesystem::path& aFilename,
                                                    const std::set<std::string>& aExcludedBlocks = {})
    -> std::unique_ptr<::krino::MeshInterface>;

/// @brief Takes a LevelSetPrimitives specification @a aLevelSetPrimitives along with a bulk data @a aBulkData from a
/// KrinoMesh and sets the level set field based on the primitives specified required fields and other setup for cutting
/// @pre the environment for krino was initialized by calling 'initialize_environment_for_krino', followed by
/// 'read_and_setup_for_decomposition' to setup the level set fields in the mesh where the bulk data came from.
[[nodiscard]] auto make_level_set_field_from_primitives(const LevelSetPrimitives& aLevelSetPrimitives,
                                                        const stk::mesh::BulkData& aBulkData)
    -> std::vector<::krino::LS_Field>;

/// @brief Returns all level-set fields defined on @a aKrinoMesh.
/// @pre The environment for krino was initialized by calling 'initialize_environment_for_krino', followed by
/// 'read_and_setup_for_decomposition' to setup the level set fields in the krino mesh.
[[nodiscard]] auto get_level_set_fields(::krino::MeshInterface& aKrinoMesh) -> std::vector<::krino::LS_Field>;

/// @brief Takes a KrinoMesh @a aKrinoMesh and a level set field @a aLevelSetFields and determines the background node
/// ids in the cut mesh.
/// @pre the environment for krino was initialized by calling 'initialize_environment_for_krino', followed by
/// 'read_and_setup_for_decomposition' to setup the level set fields in the krino mesh,  followed by a call to set up
/// the level set field values, followed by a call to 'cut_mesh'.
[[nodiscard]] auto background_node_ids(const ::krino::MeshInterface& aKrinoMesh,
                                       const std::vector<::krino::LS_Field>& aLevelSetFields)
    -> std::vector<stk::mesh::EntityId>;

/// @brief Takes a KrinoMesh @a aKrinoMesh and a VoidPhase specification @a aVoidPhase and determines the nodes in the
/// cut mesh. This includes the new nodes on the cut mesh surface as well as the background nodes. Background nodes that
/// would be eliminated from a cut_mesh operation are NOT included.
/// @pre the environment for krino was initialized by calling 'initialize_environment_for_krino', followed by
/// 'read_and_setup_for_decomposition' to setup the level set fields in the krino mesh,  followed by a call to set up
/// the level set field values, followed by a call to 'cut_mesh'.
[[nodiscard]] auto cut_mesh_node_ids(const ::krino::MeshInterface& aKrinoMesh, const VoidPhase aVoidPhase)
    -> std::vector<stk::mesh::EntityId>;

/// @brief Take a krino mesh's bulk data @a aBulkData along with a level set field specification @a aLevelSetFields and
/// perform the cutting of the mesh and creation of child nodes. Each child node generated in the cut operation have
/// only two parents and those parents are background nodes. The reverse procedure for cut_mesh is:
///    ::krino::CDMesh::reset_mesh_to_original_undecomposed_state(mKrinoMesh->bulk_data());
/// @pre the environment for krino was initialized by calling 'initialize_environment_for_krino', followed by
/// 'read_and_setup_for_decomposition' to setup the level set fields in the krino mesh, followed by a call to set up the
/// level set field values
void cut_mesh(stk::mesh::BulkData& aBulkData,
              const std::vector<::krino::LS_Field>& aLevelSetFields,
              const SnappingParameters aSnappingParameters);

/// @brief Take a level set field vector @a aLevelSetFields and a node @a aNode and return the data stored in the mesh
/// @pre the environment for krino was initialized by calling 'initialize_environment_for_krino', followed by
/// 'read_and_setup_for_decomposition'
template <typename LevelSetFieldVector>
[[nodiscard]] auto level_set_value(LevelSetFieldVector&& aLevelSetFields, const stk::mesh::Entity& aNode)
    -> decltype(auto);

/// @brief Simple accessor to grab the name of the level set field specified in a mesh.
[[nodiscard]] auto get_level_set_field_name() -> std::string;

/// @brief Given a KrinoMesh @a aKrinoMesh and its associated level set fields @a aLevelSetFields, transform these
/// values into an unordered map of global id to double.
[[nodiscard]] auto get_level_set_values(const ::krino::MeshInterface& aKrinoMesh,
                                        const std::vector<::krino::LS_Field>& aLevelSetFields)
    -> std::unordered_map<stk::mesh::EntityId, double>;

/// @brief Take a krino mesh @a aKrinoMesh along with a level set field specification @a aLevelSetFields and
/// return the background nodes after a cut operation.
/// @pre the environment for krino was initialized by calling 'initialize_environment_for_krino', followed by
/// 'read_and_setup_for_decomposition' to setup the level set fields in the krino mesh, followed by a call to set up the
/// level set field values
[[nodiscard]] auto node_entities_in_mesh(const ::krino::MeshInterface& aKrinoMesh,
                                         const std::vector<::krino::LS_Field>& aLevelSetFields)
    -> stk::mesh::EntityVector;

/// @brief Take a krino mesh's bulk data @a aBulkData and write it to the file specified in @a aOutputFileName. The void
/// phase @a aVoidPhase specifies whether to include the void region or not when writing to disk.
/// @pre the environment for krino was initialized by calling 'initialize_environment_for_krino', followed by
/// 'read_and_setup_for_decomposition' to setup the level set fields in the krino mesh,  followed by a call to set up
/// the level set field values, followed by a call to 'cut_mesh'.
void write_mesh(const stk::mesh::BulkData& aBulkData,
                const std::filesystem::path& aOutputFileName,
                const VoidPhase aVoidPhase);

template <typename LevelSetFieldVector>
[[nodiscard]] auto level_set_value(LevelSetFieldVector&& aLevelSetFields,
                                   const stk::mesh::Entity& aNode) -> decltype(auto)
{
    assert(!aLevelSetFields.empty());
    return *::krino::field_data<double>(aLevelSetFields.front().isovar, aNode);
}

}  // namespace plato::third_party_integration::krino

#endif
