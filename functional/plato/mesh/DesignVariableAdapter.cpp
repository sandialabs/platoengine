#include "plato/geometry/library/DesignVariableAdapter.hpp"

#include "plato/mesh/DesignVariableConversion.hpp"

namespace plato::geometry::library
{
namespace
{
const auto tIdentityJacobian =
    linear_algebra::JacobianMultiplier{[](const linear_algebra::DynamicVector<double>& aVector) { return aVector; }};
}

auto design_variables_to_analysis_mesh_adapter(const mesh::Mesh& aMesh) -> DesignVariableToAnalysisMeshAdapterFunction
{
    return DesignVariableToAnalysisMeshAdapterFunction{
        [aMesh](const linear_algebra::DynamicVector<double>& aNodalField)
        {
            return mesh::DesignVariablesConversion{aMesh}.nodalFieldToAnalysisDomainMesh(
                mesh::NodalFieldVectorReference{aNodalField.stdVector()});
        },
        [](const linear_algebra::DynamicVector<double>&) { return tIdentityJacobian; },
        [](const linear_algebra::DynamicVector<double>&) { return tIdentityJacobian; }};
}

auto analysis_mesh_to_design_variables_adapter(const mesh::Mesh& aMesh) -> AnalysisMeshToDesignVariablesAdapterFunction
{
    return AnalysisMeshToDesignVariablesAdapterFunction{
        [aMesh](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        {
            return linear_algebra::DynamicVector<double>{mesh::DesignVariablesConversion{aMesh}
                                                             .analysisDomainMeshToNodalFieldVector(aAnalysisDomainMesh)
                                                             .mValue};
        },
        [](const analysis::AnalysisDomainMesh&) { return tIdentityJacobian; },
        [](const analysis::AnalysisDomainMesh&) { return tIdentityJacobian; }};
}

}  // namespace plato::geometry::library
