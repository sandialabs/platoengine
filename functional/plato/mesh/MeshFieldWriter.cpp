#include "plato/mesh/MeshFieldWriter.hpp"

#include "plato/design_variables/MeshDesignVariablesRandomAccessView.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/third_party_integration/stk_io/IOUtilities.hpp"

namespace plato::mesh
{
MeshFieldWriter::MeshFieldWriter(Mesh aMeshBase) : Mesh{std::move(aMeshBase)} {}

void MeshFieldWriter::writeNodalField(const std::filesystem::path& aFilePath,
                                      const NodalFieldVectorReference& aScalarField,
                                      const std::string_view aFieldName,
                                      const double aFixedValue) const
{
    const auto tDesignVariables = DesignVariablesConversion{*this}.nodalFieldToMeshDesignVariables(aScalarField);
    writeMeshDesignVariables(aFilePath, tDesignVariables, aFieldName, aFixedValue);
}

void MeshFieldWriter::writeElementField(const std::filesystem::path& aFilePath,
                                        const ElementFieldVectorReference& aScalarField,
                                        const std::string_view aFieldName,
                                        const double aFixedValue) const
{
    const auto tDesignVariables = DesignVariablesConversion{*this}.elementFieldToMeshDesignVariables(aScalarField);
    writeMeshDesignVariables(aFilePath, tDesignVariables, aFieldName, aFixedValue);
}

void MeshFieldWriter::writeMeshDesignVariables(const std::filesystem::path& aFilePath,
                                               const design_variables::MeshDesignVariables& aMeshDesignVariables,
                                               const std::string_view aFieldName,
                                               const double aFixedValue) const
{
    namespace tpi = plato::third_party_integration;

    const auto tScalarField =
        [tDesignVariablesView = design_variables::MeshDesignVariablesRandomAccessView{aMeshDesignVariables},
         tFixedScalarField = design_variables::ScalarFieldValue{0, 0, aFixedValue}](const std::size_t aGlobalIndex)
    { return tDesignVariablesView[aGlobalIndex].value_or(tFixedScalarField).mValue; };

    if (EntityCounts{*this}.areElementDesignVariables(aMeshDesignVariables))
    {
        tpi::stk_io::write_element_scalar_field(filePath(), tScalarField, aFieldName, aFilePath);
    }
    else
    {
        tpi::stk_io::write_nodal_scalar_field(filePath(), tScalarField, aFieldName, aFilePath);
    }
}

}  // namespace plato::mesh
