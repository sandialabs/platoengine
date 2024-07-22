#include "plato/mesh/Mesh.hpp"

#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::mesh
{
/// @brief Loads a mesh from disk at the path @a aMeshName
Mesh::Mesh(const std::filesystem::path& aMeshName)
    : mFilePath{aMeshName}, mBulk{third_party_integration::stk_io::read_mesh_bulk_data(aMeshName)}
{
}

const std::filesystem::path& Mesh::filePath() const { return mFilePath; }

const stk::mesh::BulkData& Mesh::bulkData() const
{
    assert(mBulk);
    return *mBulk;
}

}  // namespace plato::mesh
