#include "plato/mesh/MeshFieldAppender.hpp"

#include <stk_io/StkMeshIoBroker.hpp>

#include "plato/analysis/AnalysisDomainMeshRandomAccessView.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/MeshFieldOutputUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"

namespace plato::mesh
{
MeshFieldAppender::MeshFieldAppender(Mesh aMeshBase, const double aTimeStep)
    : MeshOutput{std::move(aMeshBase)},
      mTimeStep{aTimeStep},
      mMeshIOBroker{plato::third_party_integration::stk_io::create_io_broker_from_bulk(bulkData())},
      mFileHandle{mMeshIOBroker->create_output_mesh(filePath(), stk::io::APPEND_RESULTS)}
{
}

MeshFieldAppender::~MeshFieldAppender()
{
    plato::third_party_integration::stk_io::write_fields_at_time(*mMeshIOBroker, mFileHandle, mTimeStep);
}

void MeshFieldAppender::addFieldOnAnalysisDomainMesh(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                     const std::string_view aFieldName,
                                                     const double aFixedValue)
{
    MeshFieldType tFieldType = EntityCounts{*this}.areElementDesignVariables(aAnalysisDomainMesh)
                                   ? MeshFieldType::kElement
                                   : MeshFieldType::kNode;
    update_output_field(*mMeshIOBroker, mFileHandle, tFieldType, aFieldName, aAnalysisDomainMesh, aFixedValue);
}

}  // namespace plato::mesh
