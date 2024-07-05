#ifndef PLATO_MESH_MESH
#define PLATO_MESH_MESH

#include <filesystem>
#include <memory>
#include <vector>

namespace stk::mesh
{
// In trilinos 15.1, there are conflicting forward declarations of BulkData, of which clang-tidy disapproves.
class BulkData;  // NOLINT
}  // namespace stk::mesh

namespace plato::third_party_integration::stk_io
{
struct CommandGenerator;
}

namespace plato::mesh
{
/// @brief Provides common mesh operations such as retrieving the number of elements or nodal coordinates.
class Mesh
{
   public:
    Mesh(const std::filesystem::path& aMeshName);
    Mesh(const third_party_integration::stk_io::CommandGenerator& aMeshName);

    [[nodiscard]] unsigned int numberOfElements() const;
    [[nodiscard]] unsigned int numberOfNodes() const;
    [[nodiscard]] unsigned int spatialDimensions() const;
    [[nodiscard]] std::vector<double> flattenedNodalCoordinates() const;

    void writeMesh(const std::filesystem::path& aOutputFileName) const;

   private:
    std::shared_ptr<stk::mesh::BulkData> mBulk;
};

}  // namespace plato::mesh

#endif
