#include "plato/mesh/MeshFieldWriter.hpp"

#include "plato/analysis/AnalysisDomainMeshRandomAccessView.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"

namespace plato::mesh
{
MeshFieldWriter::MeshFieldWriter(Mesh aMeshBase) : Mesh{std::move(aMeshBase)} {}

void MeshFieldWriter::writeNodalField(const std::filesystem::path& aFilePath,
                                      const NodalFieldVectorReference& aScalarField,
                                      const std::string_view aFieldName,
                                      const double aFixedValue) const
{
    const auto tDesignVariables = DesignVariablesConversion{*this}.nodalFieldToAnalysisDomainMesh(aScalarField);
    writeAnalysisDomainMesh(aFilePath, tDesignVariables, aFieldName, aFixedValue);
}

void MeshFieldWriter::writeElementField(const std::filesystem::path& aFilePath,
                                        const ElementFieldVectorReference& aScalarField,
                                        const std::string_view aFieldName,
                                        const double aFixedValue) const
{
    const auto tDesignVariables = DesignVariablesConversion{*this}.elementFieldToAnalysisDomainMesh(aScalarField);
    writeAnalysisDomainMesh(aFilePath, tDesignVariables, aFieldName, aFixedValue);
}

void MeshFieldWriter::writeAnalysisDomainMesh(const std::filesystem::path& aFilePath,
                                              const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                              const std::string_view aFieldName,
                                              const double aFixedValue) const
{
    namespace tpi = plato::third_party_integration;

    const auto tScalarField =
        [tDesignVariablesView = analysis::AnalysisDomainMeshRandomAccessView{aAnalysisDomainMesh},
         tFixedScalarField = analysis::ScalarFieldValue{0, 0, aFixedValue}](const std::size_t aGlobalIndex)
    { return tDesignVariablesView[aGlobalIndex].value_or(tFixedScalarField).mValue; };

    if (EntityCounts{*this}.areElementDesignVariables(aAnalysisDomainMesh))
    {
        tpi::stk_io::write_element_scalar_field(filePath(), tScalarField, aFieldName, aFilePath);
    }
    else
    {
        tpi::stk_io::write_nodal_scalar_field(filePath(), tScalarField, aFieldName, aFilePath);
    }
}

}  // namespace plato::mesh
