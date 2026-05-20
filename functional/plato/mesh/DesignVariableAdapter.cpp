#include "plato/mesh/DesignVariableAdapter.hpp"

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/Mesh.hpp"

namespace plato::mesh
{
namespace
{
const auto kIdentityJacobian =
    linear_algebra::JacobianMultiplier{[](const linear_algebra::DynamicVector<double>& aVector) { return aVector; }};
const auto kIdentityAdjointJacobian = linear_algebra::AdjointJacobianMultiplier{kIdentityJacobian};
}  // namespace

auto design_variables_to_analysis_mesh_adapter(const mesh::Mesh& aMesh) -> DesignVariableToAnalysisMeshAdapterFunction
{
    return DesignVariableToAnalysisMeshAdapterFunction{
        [aMesh](const linear_algebra::DynamicVector<double>& aNodalField)
        {
            return mesh::DesignVariablesConversion{aMesh}.nodalFieldToAnalysisDomainMesh(
                mesh::NodalFieldVectorReference{aNodalField.stdVector()});
        },
        [](const linear_algebra::DynamicVector<double>&) { return kIdentityJacobian; },
        [](const linear_algebra::DynamicVector<double>&) { return kIdentityAdjointJacobian; }};
}

auto analysis_mesh_to_design_variables_adapter(const mesh::Mesh& aMesh) -> AnalysisMeshToDesignVariablesAdapterFunction
{
    return AnalysisMeshToDesignVariablesAdapterFunction{
        [aMesh](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        {
            return linear_algebra::DynamicVector<double>{
                mesh::DesignVariablesConversion::analysisDomainMeshToNodalFieldVector(aAnalysisDomainMesh).mValue};
        },
        [](const analysis::AnalysisDomainMesh&) { return kIdentityJacobian; },
        [](const analysis::AnalysisDomainMesh&) { return kIdentityAdjointJacobian; }};
}

}  // namespace plato::mesh
