#include "plato/geometry/extension/OutputUtilities.hpp"

#include <cassert>
#include <memory>

#include "plato/geometry/library/OutputInfo.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/MeshOutput.hpp"

namespace plato::geometry::extension
{
auto output_nodal_field(const MeshFieldOutputInfo& aMeshOutputInfo,
                        const filter::library::FilterFunction& aFilterFunction,
                        const linear_algebra::DynamicVector<double>& aSolution,
                        const library::OutputInfo& aOutputInfo) -> analysis::AnalysisDomainMesh
{
    const auto tMeshOutput = mesh::make_mesh_output(
        mesh::output_mode(aOutputInfo.mOverwrite), mesh::InputFilePath{aMeshOutputInfo.mInputMesh.filePath()},
        mesh::OutputFilePath{aMeshOutputInfo.mOutputPath}, aMeshOutputInfo.mFixedBlocks, aOutputInfo.mIteration);
    assert(tMeshOutput);

    const auto tNodalDesignParameters =
        mesh::DesignVariablesConversion{aMeshOutputInfo.mInputMesh}.nodalFieldToAnalysisDomainMesh(
            mesh::NodalFieldVectorReference{aSolution.stdVector()});

    auto tFilteredDesignParameters = aFilterFunction.evaluate<core::evaluation::kFunction>(tNodalDesignParameters);
    tMeshOutput->addFieldOnAnalysisDomainMesh(tFilteredDesignParameters, aMeshOutputInfo.mFilteredFieldName,
                                              aMeshOutputInfo.mFixedFieldValue);
    tMeshOutput->addFieldOnAnalysisDomainMesh(tNodalDesignParameters, aMeshOutputInfo.mControlFieldName,
                                              aMeshOutputInfo.mFixedFieldValue);
    return tFilteredDesignParameters;
}
}  // namespace plato::geometry::extension
