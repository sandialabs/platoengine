#ifndef PLATO_UTILITIES_STKUTILITIES
#define PLATO_UTILITIES_STKUTILITIES

#include <filesystem>
#include <memory>
#include <stk_mesh/base/BulkData.hpp>
#include <string_view>
#include <vector>

namespace plato::utilities
{
[[nodiscard]] std::shared_ptr<stk::mesh::BulkData> create_mesh(const std::string_view aGenerationCommand);

void write_mesh(const std::filesystem::path& aMeshName, std::shared_ptr<stk::mesh::BulkData> aBulk);

[[nodiscard]] std::shared_ptr<stk::mesh::BulkData> read_mesh_bulk_data(const std::filesystem::path& aMeshName);

[[nodiscard]] unsigned int element_size(const std::filesystem::path& aMeshName);

[[nodiscard]] unsigned int read_mesh_node_size(const std::filesystem::path& aMeshName);

[[nodiscard]] unsigned int node_size(const stk::mesh::BulkData& aBulk);

[[nodiscard]] unsigned int element_size(const stk::mesh::BulkData& aBulk);

[[nodiscard]] unsigned int spatial_dimensions(const stk::mesh::BulkData& aBulk);

[[nodiscard]] std::vector<double> nodal_coordinates(const stk::mesh::BulkData& aBulk);

[[nodiscard]] std::vector<double> read_mesh_density(const std::filesystem::path& aMeshName);

[[nodiscard]] std::vector<unsigned int> extract_global_node_ids(const std::string& aFilename);

void write_mesh_density(const std::filesystem::path& aInputMeshName,
                        const std::vector<double>& aDensity,
                        const std::filesystem::path& aOutputMeshName);

}  // namespace plato::utilities

#endif
