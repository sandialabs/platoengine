#include "plato/mesh/MeshOutput.hpp"

#include "plato/mesh/MeshFieldAppender.hpp"
#include "plato/mesh/MeshFieldWriter.hpp"

namespace plato::mesh
{

auto output_mode(bool aOverwrite) -> OutputMode { return aOverwrite ? OutputMode::kOverwrite : OutputMode::kAppend; }

MeshOutput::MeshOutput(Mesh aMeshBase) : Mesh{std::move(aMeshBase)} {}

auto make_mesh_output(const OutputMode aOutputMode,
                      const InputFilePath& aInputFilePath,
                      const OutputFilePath& aOutputFilePath,
                      const std::set<std::string>& aFixedBlocks,
                      const std::size_t aTimeStep) -> std::unique_ptr<MeshOutput>
{
    const auto tTimeStepAsDouble = static_cast<double>(aTimeStep);
    switch (aOutputMode)
    {
        case OutputMode::kOverwrite:
            return std::make_unique<MeshFieldWriter>(Mesh{aInputFilePath.mValue, aFixedBlocks}, aOutputFilePath.mValue,
                                                     tTimeStepAsDouble);
        case OutputMode::kAppend:
            return std::make_unique<MeshFieldAppender>(Mesh{aOutputFilePath.mValue, aFixedBlocks}, tTimeStepAsDouble);
    }
    return nullptr;
}
}  // namespace plato::mesh
