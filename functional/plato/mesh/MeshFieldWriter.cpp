#include "plato/mesh/MeshFieldWriter.hpp"

#include "plato/mesh/EntityCounts.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::mesh
{
MeshFieldWriter::MeshFieldWriter(Mesh aMeshBase) : Mesh{std::move(aMeshBase)} {}

void MeshFieldWriter::writeNodalField(const std::filesystem::path& aFilePath,
                                      const NodalFieldVectorReference& aScalarField,
                                      const std::string_view aFieldName,
                                      const double aFixedValue) const
{
    namespace tpi = plato::third_party_integration;
    const auto tDesignVariablesConversion = DesignVariablesConversion{*this};
    const auto tNodalField = tpi::stk_io::ScalarField{tDesignVariablesConversion.nodalFieldToNodalIDMap(aScalarField),
                                                      std::string{aFieldName}, aFixedValue};
    tpi::stk_io::write_nodal_scalar_field(filePath(), tNodalField, aFilePath);
}

void MeshFieldWriter::writeElementField(const std::filesystem::path& aFilePath,
                                        const ElementFieldVectorReference& aScalarField,
                                        const std::string_view aFieldName,
                                        const double aFixedValue) const
{
    namespace tpi = plato::third_party_integration;
    const auto tDesignVariablesConversion = DesignVariablesConversion{*this};
    const auto tElementField = tpi::stk_io::ScalarField{
        tDesignVariablesConversion.elementFieldToElementIDMap(aScalarField), std::string{aFieldName}, aFixedValue};
    tpi::stk_io::write_element_scalar_field(filePath(), tElementField, aFilePath);
}

void MeshFieldWriter::writeMeshDesignVariables(const std::filesystem::path& aFilePath,
                                               const MeshDesignVariables& aMeshDesignVariables,
                                               const std::string_view aFieldName,
                                               const double aFixedValue) const
{
    namespace tpi = plato::third_party_integration;
    const auto tDesignVariablesConversion = DesignVariablesConversion{*this};
    const auto tScalarField =
        tpi::stk_io::ScalarField{tDesignVariablesConversion.meshDesignVariablesToIDMap(aMeshDesignVariables),
                                 std::string{aFieldName}, aFixedValue};
    if (EntityCounts{*this}.areElementDesignVariables(aMeshDesignVariables))
    {
        tpi::stk_io::write_element_scalar_field(filePath(), tScalarField, aFilePath);
    }
    else
    {
        tpi::stk_io::write_nodal_scalar_field(filePath(), tScalarField, aFilePath);
    }
}

}  // namespace plato::mesh
