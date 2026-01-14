#include "plato/geometry/extension/OutputUtilities.hpp"

#include <boost/mpi/communicator.hpp>
#include <cassert>
#include <memory>

#include "plato/mesh/MeshOutput.hpp"
#include "plato/output/OutputInfo.hpp"
#include "plato/utilities/MPIUtilities.hpp"

namespace plato::geometry::extension
{
namespace
{
void output_field_impl(const MeshFieldOutputInfo& aMeshFieldOutputInfo)
{
    const auto tMeshOutput = mesh::mesh_output(
        mesh::output_mode(aMeshFieldOutputInfo.mOutputInfo.mOverwrite), aMeshFieldOutputInfo.mSourceAnalysisDomainMesh,
        aMeshFieldOutputInfo.mOutputPath, aMeshFieldOutputInfo.mOutputInfo.mIteration);

    assert(tMeshOutput);
    tMeshOutput->addFieldFromAnalysisDomainMesh(aMeshFieldOutputInfo.mSourceAnalysisDomainMesh,
                                                aMeshFieldOutputInfo.mFieldName, aMeshFieldOutputInfo.mFixedFieldValue);
}
}  // namespace

void output_field(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                  const std::filesystem::path& aRestartFileName,
                  const std::string_view aFieldName,
                  const double aFixedValue,
                  const output::OutputInfo& aOutputInfo)
{
    const auto tMeshFieldOutputInfo = MeshFieldOutputInfo{.mOutputPath = aRestartFileName,
                                                          .mSourceAnalysisDomainMesh = std::cref(aAnalysisDomainMesh),
                                                          .mFieldName = aFieldName,
                                                          .mFixedFieldValue = aFixedValue,
                                                          .mOutputInfo = aOutputInfo};
    utilities::execute_on_root(boost::mpi::communicator{},
                               [&tMeshFieldOutputInfo]() { output_field_impl(tMeshFieldOutputInfo); });
}

}  // namespace plato::geometry::extension
