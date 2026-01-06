#include "plato/geometry/extension/OutputUtilities.hpp"

#include <boost/mpi/communicator.hpp>
#include <cassert>
#include <memory>

#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/MeshOutput.hpp"
#include "plato/output/OutputInfo.hpp"
#include "plato/utilities/MPIUtilities.hpp"

namespace plato::geometry::extension
{
auto output_nodal_field(const MeshFieldOutputInfo& aMeshOutputInfo,
                        const filter::library::FilterFunction& aFilterFunction,
                        const linear_algebra::DynamicVector<double>& aSolution,
                        const output::OutputInfo& aOutputInfo) -> analysis::AnalysisDomainMesh
{
    const auto tNodalDesignParameters =
        mesh::DesignVariablesConversion{aMeshOutputInfo.mInputMesh}.nodalFieldToAnalysisDomainMesh(
            mesh::NodalFieldVectorReference{aSolution.stdVector()});
    auto tFilteredDesignParameters = aFilterFunction.evaluate<core::evaluation::kFunction>(tNodalDesignParameters);

    utilities::execute_on_root(
        boost::mpi::communicator{},
        [&aMeshOutputInfo, &aOutputInfo, &tFilteredDesignParameters, &tNodalDesignParameters]()
        {
            const auto tMeshOutput = mesh::make_mesh_output(mesh::output_mode(aOutputInfo.mOverwrite),
                                                            mesh::InputFilePath{aMeshOutputInfo.mInputMesh.filePath()},
                                                            mesh::OutputFilePath{aMeshOutputInfo.mOutputPath},
                                                            aMeshOutputInfo.mFixedBlocks, aOutputInfo.mIteration);
            assert(tMeshOutput);

            tMeshOutput->addFieldOnAnalysisDomainMesh(tFilteredDesignParameters, aMeshOutputInfo.mFilteredFieldName,
                                                      aMeshOutputInfo.mFixedFieldValue);
            tMeshOutput->addFieldOnAnalysisDomainMesh(tNodalDesignParameters, aMeshOutputInfo.mControlFieldName,
                                                      aMeshOutputInfo.mFixedFieldValue);
        });

    return tFilteredDesignParameters;
}
}  // namespace plato::geometry::extension
