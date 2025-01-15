#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_WRITEUTILITIES
#define PLATO_THIRDPARTYINTEGRATION_STKIO_WRITEUTILITIES

#include <filesystem>
#include <functional>
#include <memory>
#include <stk_mesh/base/Types.hpp>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"

namespace stk::mesh
{
// In trilinos 15.1, there are conflicting forward declarations of BulkData, of which clang-tidy disapproves.
class BulkData;  // NOLINT
}  // namespace stk::mesh

namespace stk::io
{
class StkMeshIoBroker;
}

namespace plato::third_party_integration::stk_io
{
/// @brief Given a pathname  @a aMeshName and the Command generator @a aCommandGenerator, write to disk the data in
/// exodus format
void write_mesh(const std::filesystem::path& aMeshName, const CommandGenerator& aCommandGenerator);

/// @brief Given a pathname  @a aMeshName and mesh description (e.g. `textmesh`), write to disk the data in
/// exodus format
void write_mesh(const std::filesystem::path& aMeshName, std::string_view aMeshDescription);

/// @brief Use a STK @a aGenerationCommand, e.g., "generated:1x1x1" to create and return a shared pointer to a STK Bulk
[[nodiscard]] std::shared_ptr<stk::mesh::BulkData> generate_bulk_data(const CommandGenerator& aCommandGenerator);

/// @brief Given a pathname  @a aMeshName and the STK Bulk data @a aBulk, write to disk the data in exodus format
void write_bulk_data(const std::filesystem::path& aMeshName, std::shared_ptr<stk::mesh::BulkData> aBulk);

/// @brief Creates a StkMeshIoBroker by reading the contents of the file at @a aInputMeshPath.
///
/// The created object may then be used to add fields using other utility functions. create_output_mesh must be called
/// to set up a new output file and finalize_mesh_data must be called to close the file and write any field data.
/// @sa create_output_mesh
/// @sa write_nodal_scalar_field
/// @sa write_element_scalar_field
/// @sa finalize_mesh_data
[[nodiscard]] auto create_io_mesh_broker(const std::filesystem::path& aInputMeshPath)
    -> std::unique_ptr<stk::io::StkMeshIoBroker>;

/// @brief Creates an output mesh at file at the file path @a aOutputMeshPath.
/// @warning If a file exists at @a aOutputMeshPath, it will be overwritten.
/// @return A file handle that must be used to reference the file in subsequent calls to write data.
[[nodiscard]] auto create_output_mesh(const std::filesystem::path& aOutputMeshPath, stk::io::StkMeshIoBroker& aIOBroker)
    -> std::size_t;

/// @brief A function for retrieving a scalar field value using an index into some data structure.
using ScalarFieldFunction = std::function<double(std::size_t)>;

/// @brief Writes the data in @a aScalarField to the mesh associated with @a aIOBroker and @a aFileHandle using a field
/// name @a aFieldName.
void write_nodal_scalar_field(stk::io::StkMeshIoBroker& aIOBroker,
                              const ScalarFieldFunction& aScalarField,
                              const std::string_view aFieldName,
                              std::size_t aFileHandle);

/// @brief Writes the data in @a aScalarField to the mesh associated with @a aIOBroker and @a aFileHandle using a field
/// name @a aFieldName.
void write_element_scalar_field(stk::io::StkMeshIoBroker& aIOBroker,
                                const ScalarFieldFunction& aScalarField,
                                const std::string_view aFieldName,
                                std::size_t aFileHandle);

/// @brief This must be called to close a mesh and write its data.
void finalize_mesh_data(std::unique_ptr<stk::io::StkMeshIoBroker>&& aIOBroker, std::size_t aFileHandle);

}  // namespace plato::third_party_integration::stk_io

#endif
