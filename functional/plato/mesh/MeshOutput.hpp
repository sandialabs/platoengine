#ifndef PLATO_MESH_MESHOUTPUT
#define PLATO_MESH_MESHOUTPUT

#include <memory>
#include <set>
#include <string>
#include <string_view>

#include "plato/mesh/Mesh.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::mesh
{
/// @brief The supported types of output for meshes.
enum struct OutputMode
{
    kOverwrite,
    kAppend
};

/// @brief Returns OverwriteMode based on a bool flag specifying whether or not to overwrite.
auto output_mode(bool aOverwrite) -> OutputMode;

/// @brief Defines a common interface for outputting mesh quantities.
class MeshOutput : public Mesh
{
   public:
    MeshOutput(Mesh aMeshBase);
    virtual ~MeshOutput() = default;

    virtual void addFieldOnAnalysisDomainMesh(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                              std::string_view aFieldName,
                                              double aFixedValue) = 0;

    MeshOutput(const MeshOutput&) = delete;
    MeshOutput(MeshOutput&&) = delete;
    MeshOutput& operator=(const MeshOutput&) = delete;
    MeshOutput& operator=(MeshOutput&&) = delete;
};

using InputFilePath = utilities::NamedType<std::filesystem::path, struct InputFilePathTag>;
using OutputFilePath = utilities::NamedType<std::filesystem::path, struct OutputFilePathTag>;

/// @brief Factory function for creating the appropriate derived type of MeshOutput based on @a aOutputMode.
/// @param aOutputMode Specifies whether to overwrite a mesh file or append to an existing file.
/// @param aInputFilePath Path to an existing mesh, that will be used as the basis for the output. Not used for
/// appending.
/// @param aFixedBlocks A set of exodus block names, giving the fixed blocks in the mesh.
/// @param aOutputFilePath The path at which to output the mesh data, must exist if appending.
/// @param aTimeStep The time step at which to write the output.
[[nodiscard]] auto make_mesh_output(OutputMode aOutputMode,
                                    const InputFilePath& aInputFilePath,
                                    const OutputFilePath& aOutputFilePath,
                                    const std::set<std::string>& aFixedBlocks,
                                    std::size_t aTimeStep = 1) -> std::unique_ptr<MeshOutput>;

}  // namespace plato::mesh

#endif
