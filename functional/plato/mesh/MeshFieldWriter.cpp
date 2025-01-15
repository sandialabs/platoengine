#include "plato/mesh/MeshFieldWriter.hpp"

#include <stk_io/StkMeshIoBroker.hpp>

#include "plato/analysis/AnalysisDomainMeshRandomAccessView.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"

namespace plato::mesh
{
MeshFieldWriter::MeshFieldWriter(Mesh aMeshBase, const std::filesystem::path& aMeshFilePath)
    : Mesh{std::move(aMeshBase)},
      mMeshIOBroker{third_party_integration::stk_io::create_io_mesh_broker(filePath())},
      mFileHandle{third_party_integration::stk_io::create_output_mesh(aMeshFilePath, *mMeshIOBroker)}
{
}

MeshFieldWriter::~MeshFieldWriter()
{
    third_party_integration::stk_io::finalize_mesh_data(std::move(mMeshIOBroker), mFileHandle);
}

void MeshFieldWriter::addNodalField(const NodalFieldVectorReference& aScalarField,
                                    const std::string_view aFieldName,
                                    const double aFixedValue)
{
    const auto tDesignVariables = DesignVariablesConversion{*this}.nodalFieldToAnalysisDomainMesh(aScalarField);
    addAnalysisDomainMesh(tDesignVariables, aFieldName, aFixedValue);
}

void MeshFieldWriter::addElementField(const ElementFieldVectorReference& aScalarField,
                                      const std::string_view aFieldName,
                                      const double aFixedValue)
{
    const auto tDesignVariables = DesignVariablesConversion{*this}.elementFieldToAnalysisDomainMesh(aScalarField);
    addAnalysisDomainMesh(tDesignVariables, aFieldName, aFixedValue);
}

void MeshFieldWriter::addAnalysisDomainMesh(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                            const std::string_view aFieldName,
                                            const double aFixedValue)
{
    namespace tpi = plato::third_party_integration;

    const auto tScalarField =
        [tDesignVariablesView = analysis::AnalysisDomainMeshRandomAccessView{aAnalysisDomainMesh},
         tFixedScalarField = analysis::ScalarFieldValue{0, 0, aFixedValue}](const std::size_t aGlobalIndex)
    { return tDesignVariablesView[aGlobalIndex].value_or(tFixedScalarField).mValue; };

    if (EntityCounts{*this}.areElementDesignVariables(aAnalysisDomainMesh))
    {
        tpi::stk_io::write_element_scalar_field(*mMeshIOBroker, tScalarField, aFieldName, mFileHandle);
    }
    else
    {
        tpi::stk_io::write_nodal_scalar_field(*mMeshIOBroker, tScalarField, aFieldName, mFileHandle);
    }
}

}  // namespace plato::mesh
