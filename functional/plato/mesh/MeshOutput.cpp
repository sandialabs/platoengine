#include "plato/mesh/MeshOutput.hpp"

#include <cassert>
#include <cstddef>
#include <filesystem>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/mesh/MeshFieldAppender.hpp"
#include "plato/mesh/MeshFieldWriter.hpp"

namespace plato::mesh
{

auto output_mode(bool aOverwrite) -> OutputMode { return aOverwrite ? OutputMode::kOverwrite : OutputMode::kAppend; }

MeshOutput::MeshOutput(Mesh aMeshBase) : Mesh{std::move(aMeshBase)} {}

auto mesh_output(OutputMode aOutputMode,
                 analysis::AnalysisDomainMesh aSourceAnalysisDomainMesh,
                 const std::filesystem::path& aOutputFilePath,
                 const std::size_t aTimeStep) -> std::unique_ptr<MeshOutput>
{
    if (aOutputMode == OutputMode::kOverwrite || !std::filesystem::exists(aOutputFilePath))
    {
        return detail::overwrite_mesh_output(aSourceAnalysisDomainMesh, aOutputFilePath, aTimeStep);
    }
    aSourceAnalysisDomainMesh.mFileName = aOutputFilePath;
    return detail::append_mesh_output(aSourceAnalysisDomainMesh, aTimeStep);
}

namespace detail
{
auto overwrite_mesh_output(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                           const std::filesystem::path& aOutputFilePath,
                           const std::size_t aTimeStep) -> std::unique_ptr<MeshOutput>
{
    const auto tTimeStepAsDouble = static_cast<double>(aTimeStep);
    return std::make_unique<MeshFieldWriter>(Mesh{aAnalysisDomainMesh}, aOutputFilePath, tTimeStepAsDouble);
}

auto append_mesh_output(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh, const std::size_t aTimeStep)
    -> std::unique_ptr<MeshOutput>
{
    const auto tTimeStepAsDouble = static_cast<double>(aTimeStep);
    assert(std::filesystem::exists(aAnalysisDomainMesh.mFileName));
    return std::make_unique<MeshFieldAppender>(Mesh{aAnalysisDomainMesh}, tTimeStepAsDouble);
}
}  // namespace detail
}  // namespace plato::mesh
