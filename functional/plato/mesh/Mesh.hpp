#ifndef PLATO_MESH_MESH
#define PLATO_MESH_MESH

#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

#include "plato/mesh/MeshProxy.hpp"
#include "plato/third_party_integration/common/BlockData.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"

namespace stk::mesh
{
// In trilinos 15.1, there are conflicting forward declarations of BulkData, of which clang-tidy disapproves.
class BulkData;  // NOLINT
}  // namespace stk::mesh

namespace plato::mesh
{
class Mesh
{
   public:
    /// @brief Loads a mesh from disk at the path @a aMeshName
    explicit Mesh(const std::filesystem::path& aMeshName);

    /// @brief Returns the path to the mesh on disk.
    const std::filesystem::path& filePath() const;

   protected:
    /// @brief Returns a reference to the underlying BulkData.
    const stk::mesh::BulkData& bulkData() const;

   private:
    std::filesystem::path mFilePath;
    std::shared_ptr<stk::mesh::BulkData> mBulk;
};

}  // namespace plato::mesh

#endif
