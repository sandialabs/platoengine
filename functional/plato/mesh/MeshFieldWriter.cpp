#include "plato/mesh/MeshFieldWriter.hpp"

#include <filesystem>
#include <stk_io/DatabasePurpose.hpp>
#include <stk_io/StkMeshIoBroker.hpp>
#include <string_view>
#include <utility>

#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshFieldOutputUtilities.hpp"
#include "plato/mesh/MeshOutput.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"

namespace plato::mesh
{
namespace
{
void initialize_field(stk::io::StkMeshIoBroker& aIOBroker,
                      const EntityCounts& aEntityCounts,
                      const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                      const std::string_view aFieldName)
{
    namespace tpi = plato::third_party_integration;

    if (aEntityCounts.areElementDesignVariables(aAnalysisDomainMesh))
    {
        tpi::stk_io::initialize_element_scalar_field(aIOBroker, aFieldName);
    }
    else
    {
        tpi::stk_io::initialize_nodal_scalar_field(aIOBroker, aFieldName);
    }
}
}  // namespace

MeshFieldWriter::MeshFieldWriter(Mesh aMeshBase, const std::filesystem::path& aWriteFilePath, const double aTimeStep)
    : MeshOutput{std::move(aMeshBase)},
      mTimeStep{aTimeStep},
      mMeshIOBroker{plato::third_party_integration::stk_io::create_io_broker_from_input_file(filePath())},
      mFileHandle{mMeshIOBroker->create_output_mesh(aWriteFilePath, stk::io::WRITE_RESULTS)}
{
}

MeshFieldWriter::~MeshFieldWriter()
{
    plato::third_party_integration::stk_io::write_fields_at_time(*mMeshIOBroker, mFileHandle, mTimeStep);
}

void MeshFieldWriter::addFieldFromAnalysisDomainMesh(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                     const std::string_view aFieldName,
                                                     const double aFixedValue)
{
    const auto tMeshEntityCounts = EntityCounts{copy_as_mesh(*this)};
    initialize_field(*mMeshIOBroker, tMeshEntityCounts, aAnalysisDomainMesh, aFieldName);

    const auto tFieldType = tMeshEntityCounts.areElementDesignVariables(aAnalysisDomainMesh) ? MeshFieldType::kElement
                                                                                             : MeshFieldType::kNode;

    update_output_field(*mMeshIOBroker, mFileHandle, tFieldType, aFieldName, aAnalysisDomainMesh, aFixedValue);
}

}  // namespace plato::mesh
