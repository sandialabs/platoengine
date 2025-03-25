#ifndef PLATO_MESH_MESHFIELDWRITER
#define PLATO_MESH_MESHFIELDWRITER

#include <filesystem>
#include <memory>
#include <string_view>

#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshOutput.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace stk::io
{
class StkMeshIoBroker;
}

namespace plato::mesh
{
/// @brief A mixin class for Mesh that provides functions to create an output mesh on disk and write an initial design
/// variable field to it.
///
/// This is an RAII-style class that opens a mesh on construction, writes data to that mesh using its member functions,
/// and closes and saves the mesh on destruction.
class [[nodiscard]] MeshFieldWriter : public MeshOutput
{
   public:
    MeshFieldWriter(Mesh aMeshBase, const std::filesystem::path& aWriteFilePath, double aTimeStep = 1.0);

    ~MeshFieldWriter();

    /// @brief Adds the design variables in @a aAnalysisDomainMesh to the currently managed file.
    ///
    /// The type of field (node vs. element) is determined from the number of design variables.
    void addFieldOnAnalysisDomainMesh(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                      std::string_view aFieldName,
                                      double aFixedValue) override;

    MeshFieldWriter(const MeshFieldWriter&) = delete;
    MeshFieldWriter(MeshFieldWriter&&) = delete;
    MeshFieldWriter& operator=(const MeshFieldWriter&) = delete;
    MeshFieldWriter& operator=(MeshFieldWriter&&) = delete;

   private:
    double mTimeStep;
    std::unique_ptr<stk::io::StkMeshIoBroker> mMeshIOBroker;
    std::size_t mFileHandle;
};
}  // namespace plato::mesh

#endif
