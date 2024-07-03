#ifndef PLATO_MESH_MESH
#define PLATO_MESH_MESH

#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::mesh
{

class Mesh
{
   public:
    Mesh(const std::filesystem::path& aMeshName);
    Mesh(const third_party_integration::stk_io::CommandGenerator& aMeshName);

    [[nodiscard]] unsigned int numberOfElements() const;
    [[nodiscard]] unsigned int numberOfNodes() const;
    [[nodiscard]] unsigned int spatialDimensions() const;
    [[nodiscard]] std::vector<double> flattenedNodalCoordinates() const;

    void write_mesh(const std::filesystem::path& aOutputFileName) const;

   private:
    std::shared_ptr<stk::mesh::BulkData> mBulk;
};

}  // namespace plato::mesh

#endif
