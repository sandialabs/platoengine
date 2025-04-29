#ifndef PLATO_MESH_MESHFIELDAPPENDER
#define PLATO_MESH_MESHFIELDAPPENDER

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
/// @brief A mixin class for Mesh that provides functions to append a design variable field to an existing output mesh
/// file on disk.
///
/// This is an RAII-style class that opens the output mesh on construction, writes data to that mesh using its member
/// functions, and closes and saves the mesh on destruction.
class [[nodiscard]] MeshFieldAppender : public MeshOutput
{
   public:
    MeshFieldAppender(Mesh aMeshBase, double aTimeStep);

    ~MeshFieldAppender() override;

    /// @brief Adds the design variables in @a aAnalysisDomainMesh to the currently managed file.
    ///
    /// The type of field (node vs. element) is determined from the number of design variables.
    /// @pre @a aFieldName must exist on the mesh used on construction.
    void addFieldOnAnalysisDomainMesh(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                      std::string_view aFieldName,
                                      double aFixedValue) override;

    MeshFieldAppender(const MeshFieldAppender&) = delete;
    MeshFieldAppender(MeshFieldAppender&&) = delete;
    MeshFieldAppender& operator=(const MeshFieldAppender&) = delete;
    MeshFieldAppender& operator=(MeshFieldAppender&&) = delete;

   private:
    double mTimeStep;
    std::unique_ptr<stk::io::StkMeshIoBroker> mMeshIOBroker;
    std::size_t mFileHandle;
};
}  // namespace plato::mesh

#endif
