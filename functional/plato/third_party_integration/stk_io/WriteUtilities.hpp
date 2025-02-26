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
/// to set up a new output file
[[nodiscard]] auto create_io_broker_from_input_file(const std::filesystem::path& aInputMeshPath)
    -> std::unique_ptr<stk::io::StkMeshIoBroker>;

/// @brief Creates an output mesh at file at the file path @a aOutputMeshPath.
/// @warning If a file exists at @a aOutputMeshPath, it will be overwritten.
/// @return A file handle that must be used to reference the file in subsequent calls to write data.
[[nodiscard]] auto create_output_mesh(const std::filesystem::path& aOutputMeshPath, stk::io::StkMeshIoBroker& aIOBroker)
    -> std::size_t;

/// @brief Creates a StkMeshIoBroker by reading the bulk data @a aBulk.
///
/// The created object may then be used to add fields using other utility functions.
auto create_io_broker_from_bulk(stk::mesh::BulkData& aBulk) -> std::unique_ptr<stk::io::StkMeshIoBroker>;

/// @brief Declares the field @a aFieldName and puts it on the mesh associated with @a aIOBroker
void initialize_element_scalar_field(stk::io::StkMeshIoBroker& aIOBroker, const std::string_view aFieldName);

/// @brief Declares the field @a aFieldName and puts it on the mesh associated with @a aIOBroker
void initialize_nodal_scalar_field(stk::io::StkMeshIoBroker& aIOBroker, const std::string_view aFieldName);

/// @brief A function for retrieving a scalar field value using an index into some data structure.
using ScalarFieldFunction = std::function<double(std::size_t)>;

/// @brief Writes the data in @a aScalarField to the field with name @a aFieldName on the mesh associated with @a
/// aIOBroker
void populate_element_scalar_field_values(stk::io::StkMeshIoBroker& aIOBroker,
                                          const std::string_view aFieldName,
                                          const ScalarFieldFunction& aScalarField);

/// @brief Writes the data in @a aScalarField to the field with name @a aFieldName on the mesh associated with @a
/// aIOBroker
void populate_nodal_scalar_field_values(stk::io::StkMeshIoBroker& aIOBroker,
                                        const std::string_view aFieldName,
                                        const ScalarFieldFunction& aScalarField);

/// @brief Adds the field with name @a aFieldName to the mesh associated with @a aFileHandle using @a aIOBroker
void add_nodal_field_to_output_file(stk::io::StkMeshIoBroker& aIOBroker,
                                    const size_t aFileHandle,
                                    const std::string_view aFieldName);

/// @brief Adds the field with name @a aFieldName to the mesh associated with @a aFileHandle using @a aIOBroker
void add_element_field_to_output_file(stk::io::StkMeshIoBroker& aIOBroker,
                                      const size_t aFileHandle,
                                      const std::string_view aFieldName);

/// @brief Writes all fields on the mesh associated with @a aIOBroker to the mesh associated with @a aFileHandle for
/// time step @a aOutputTime
void write_fields_at_time(stk::io::StkMeshIoBroker& aIOBroker, const size_t aFileHandle, double aOutputTime);

}  // namespace plato::third_party_integration::stk_io

#endif
