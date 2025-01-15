#ifndef PLATO_MESH_MESHFIELDWRITER
#define PLATO_MESH_MESHFIELDWRITER

#include <filesystem>
#include <string_view>

#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/Mesh.hpp"

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
/// @brief A mixin class for Mesh that provides functions to write a design variable field to disk.
///
/// This is an RAII-style class that opens a mesh on construction, writes data to that mesh using its member functions,
/// and closes and saves the mesh on destruction.
/// @warning If a file exists at path @a aOutputPath, the file is overwritten.
class MeshFieldWriter : public Mesh
{
   public:
    MeshFieldWriter(Mesh aMeshBase, const std::filesystem::path& aMeshFilePath);

    ~MeshFieldWriter();

    /// @brief Adds the nodal-based scalar field in @a aScalarField to the currently managed file.
    ///
    /// Any values associated with fixed blocks in @a aScalarField are assigned to @a aFixedValue.
    void addNodalField(const NodalFieldVectorReference& aScalarField, std::string_view aFieldName, double aFixedValue);

    /// @brief Adds the element-based scalar field in @a aScalarField to the currently managed file.
    ///
    /// Any values associated with fixed blocks in @a aScalarField are assigned to @a aFixedValue.
    void addElementField(const ElementFieldVectorReference& aScalarField,
                         std::string_view aFieldName,
                         double aFixedValue);

    /// @brief Adds the design variables in @a aAnalysisDomainMesh to the currently managed file.
    ///
    /// The type of field (node vs. element) is determined from the number of design variables.
    void addAnalysisDomainMesh(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                               std::string_view aFieldName,
                               double aFixedValue);

    MeshFieldWriter(const MeshFieldWriter&) = delete;
    MeshFieldWriter(MeshFieldWriter&&) = delete;
    MeshFieldWriter& operator=(const MeshFieldWriter&) = delete;
    MeshFieldWriter& operator=(MeshFieldWriter&&) = delete;

   private:
    std::unique_ptr<stk::io::StkMeshIoBroker> mMeshIOBroker;
    std::size_t mFileHandle;
};
}  // namespace plato::mesh

#endif
