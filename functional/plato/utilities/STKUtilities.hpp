#ifndef PLATO_UTILITIES_STKUTILITIES
#define PLATO_UTILITIES_STKUTILITIES

#include <filesystem>
#include <memory>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <string_view>
#include <vector>

namespace plato::utilities
{
namespace detail
{
constexpr std::string_view kTopologyFieldName = "topology";
}

/// @brief Use a STK @a aGenerationCommand, e.g., "generated:1x1x1" to create and return a shared pointer to a STK Bulk
[[nodiscard]] std::shared_ptr<stk::mesh::BulkData> create_mesh(const std::string_view aGenerationCommand);

/// @brief Given a pathname  @a aMeshName and the STK Bulk data @a aBulk, write to disk the data in exodus format
void write_mesh(const std::filesystem::path& aMeshName, std::shared_ptr<stk::mesh::BulkData> aBulk);

/// @brief Given a pathname  @a aMeshName, read from disk and return a shared pointer to the STK Bulk data.
[[nodiscard]] std::shared_ptr<stk::mesh::BulkData> read_mesh_bulk_data(const std::filesystem::path& aMeshName);

/// @brief Given a pathname  @a aMeshName, read from disk and return the total number of elements.
[[nodiscard]] unsigned int element_size(const std::filesystem::path& aMeshName);

/// @brief Given a pathname  @a aMeshName, read from disk and return the total number of nodes.
[[nodiscard]] unsigned int read_mesh_node_size(const std::filesystem::path& aMeshName);

/// @brief Given a STK Bulk data  @a aBulk, return the total number of nodes.
[[nodiscard]] unsigned int node_size(const stk::mesh::BulkData& aBulk);

/// @brief Given a STK Bulk data  @a aBulk, return the total number of elements.
[[nodiscard]] unsigned int element_size(const stk::mesh::BulkData& aBulk);

/// @brief Given a STK Bulk data  @a aBulk, return the spatial dimensions of the mesh.
[[nodiscard]] unsigned int spatial_dimensions(const stk::mesh::BulkData& aBulk);

/// @brief Given a STK Bulk data  @a aBulk, return a std::vector of the nodal coordinates ordered x0,y0,z0,x1,y1,z1, ...
/// For 2D, only x and y coordinates are included in the vector.
[[nodiscard]] std::vector<double> nodal_coordinates(const stk::mesh::BulkData& aBulk);

/// @brief Given a pathname  @a aMeshName, return a std::vector of the nodal densities stored in the kTopologyField name
[[nodiscard]] std::vector<double> read_mesh_density(const std::filesystem::path& aMeshName);

/// @brief Given a pathname  @a aMeshName, return a std::vector of the element densities stored in the kTopologyField
/// name
[[nodiscard]] std::vector<double> read_element_density(const std::filesystem::path& aMeshName);

/// @brief Given a pathname  @a aInputMeshName, create a new mesh on disk @a aOutputMeshName that has an additional
/// nodal field stored in the kTopologyField name and populated with the data in @a aDensity
/// @pre aDensity.size() == size<stk::topology::NODE_RANK>()
void write_mesh_density(const std::filesystem::path& aInputMeshName,
                        const std::vector<double>& aDensity,
                        const std::filesystem::path& aOutputMeshName);

/// @brief Given a pathname  @a aInputMeshName, create a new mesh on disk @a aOutputMeshName that has an additional
/// element field stored in the kTopologyField name and populated with the data in @a aDensity
/// @pre aDensity.size() == size<stk::topology::ELEMENT_RANK>()
void write_element_density(const std::filesystem::path& aInputMeshName,
                           const std::vector<double>& aDensity,
                           const std::filesystem::path& aOutputMeshName);

/// @brief Given a STK Bulk data  @a aBulk, return the STK element container
stk::mesh::EntityVector element_vector(const stk::mesh::BulkData& aBulk);

namespace detail
{

template <stk::topology::rank_t Rank>
unsigned int size(const stk::mesh::BulkData& aBulk)
{
    std::vector<size_t> tEntityCounts;
    stk::mesh::comm_mesh_counts(aBulk, tEntityCounts);
    return tEntityCounts[Rank];
}

template <stk::topology::rank_t Rank>
size_t write_mesh_density_impl(stk::io::StkMeshIoBroker& aIOBroker,
                               const std::vector<double>& aDensity,
                               const std::filesystem::path& aOutputMeshName)
{
    constexpr int tScalarFieldSize = 1;
    stk::mesh::Field<double>& tField =
        aIOBroker.meta_data().declare_field<double>(Rank, std::string{detail::kTopologyFieldName}, tScalarFieldSize);
    constexpr double tInitialValue = 0;
    stk::mesh::put_field_on_mesh(tField, aIOBroker.meta_data().universal_part(), &tInitialValue);
    aIOBroker.populate_bulk_data();

    std::vector<stk::mesh::Entity> tEntity;
    constexpr bool tSortByID = false;
    stk::mesh::get_entities(aIOBroker.bulk_data(), Rank, tEntity, tSortByID);
    assert(tEntity.size() == aDensity.size());
    for (size_t iEntity = 0; iEntity < tEntity.size(); iEntity++)
    {
        double* const tFieldData = stk::mesh::field_data(tField, tEntity[iEntity]);
        *tFieldData = aDensity[aIOBroker.bulk_data().local_id(tEntity[iEntity])];
    }

    const size_t tOutputFileIndex = aIOBroker.create_output_mesh(aOutputMeshName.string(), stk::io::WRITE_RESULTS);
    aIOBroker.write_output_mesh(tOutputFileIndex);
    aIOBroker.add_field(tOutputFileIndex, tField);

    return tOutputFileIndex;
}

}  // namespace detail

}  // namespace plato::utilities

#endif
