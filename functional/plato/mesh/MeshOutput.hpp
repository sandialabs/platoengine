#ifndef PLATO_MESH_MESHOUTPUT
#define PLATO_MESH_MESHOUTPUT

#include <cstdint>
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
enum struct OutputMode : std::uint8_t
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

    virtual void addFieldFromAnalysisDomainMesh(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                std::string_view aFieldName,
                                                double aFixedValue) = 0;

    MeshOutput(const MeshOutput&) = delete;
    MeshOutput(MeshOutput&&) = delete;
    MeshOutput& operator=(const MeshOutput&) = delete;
    MeshOutput& operator=(MeshOutput&&) = delete;
};

/// @brief Given output mode @a aOutputMode, a source mesh @a aSourceAnalysisDomainMesh, the output file @a
/// aOutputFilePath and a time step @a aTimeStep, return a MeshOutput pointer
[[nodiscard]] auto mesh_output(OutputMode aOutputMode,
                               analysis::AnalysisDomainMesh aSourceAnalysisDomainMesh,
                               const std::filesystem::path& aOutputFilePath,
                               const std::size_t aTimeStep) -> std::unique_ptr<MeshOutput>;

namespace detail
{
/// @brief return a MeshOutput ready for overwriting
[[nodiscard]] auto overwrite_mesh_output(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                         const std::filesystem::path& aOutputFilePath,
                                         const std::size_t aTimeStep) -> std::unique_ptr<MeshOutput>;

/// @brief return a MeshOutput ready for appending
[[nodiscard]] auto append_mesh_output(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                      const std::size_t aTimeStep) -> std::unique_ptr<MeshOutput>;

}  // namespace detail

}  // namespace plato::mesh

#endif
