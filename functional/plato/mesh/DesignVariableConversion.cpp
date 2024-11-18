#include "plato/mesh/DesignVariableConversion.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/mesh/MeshBlocks.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"

namespace plato::mesh
{
namespace
{
template <typename F>
auto initialize_analysis_domain_mesh_data_structures(const Mesh& aMesh, const F& aIDFunction)
    -> analysis::AnalysisDomainMesh
{
    auto tAnalysisDomainMesh = analysis::AnalysisDomainMesh{aMesh.filePath(), {}};
    for (const auto tDesignBlockOrdinal : aMesh.designBlockOrdinals())
    {
        const auto tIDs = aIDFunction(aMesh, tDesignBlockOrdinal);
        const auto tField = std::vector<double>(tIDs.size());
        const auto tBlockID = MeshBlocks{aMesh}.blockID(tDesignBlockOrdinal);
        assert(tBlockID.has_value());
        // NOLINTNEXTLINE
        tAnalysisDomainMesh.mBlockScalarField.emplace(tBlockID.value(),
                                                      analysis::combine_scalar_field_values_and_ids(tField, tIDs));
    }
    return tAnalysisDomainMesh;
}

template <typename F>
auto entity_field_to_analysis_domain_mesh(const std::vector<double>& aScalarField,
                                          const Mesh& aMesh,
                                          const F& aIDFunction) -> analysis::AnalysisDomainMesh
{
    auto tAnalysisDomainMesh = initialize_analysis_domain_mesh_data_structures(aMesh, aIDFunction);

    auto tMeshView = analysis::AnalysisDomainMeshMutableSequentialView{tAnalysisDomainMesh};
    assert(aScalarField.size() == tMeshView.size());
    std::transform(
        aScalarField.cbegin(), aScalarField.cend(), tMeshView.begin(), tMeshView.begin(),
        [tVectorIndex = analysis::ScalarFieldValue::IndexType{0}](const double aField, const auto& aFieldID) mutable
        {
            const auto& tFieldIndices = static_cast<analysis::ScalarFieldValue>(aFieldID);
            return analysis::ScalarFieldValue{tFieldIndices.mGlobalMeshEntityID, tVectorIndex++, aField};
        });

    return tAnalysisDomainMesh;
}

std::vector<double> analysis_domain_mesh_to_vector(const analysis::AnalysisDomainMesh& tAnalysisDomainMesh)
{
    const auto aAnalysisDomainMeshView = analysis::AnalysisDomainMeshSequentialView{tAnalysisDomainMesh};
    auto tNodalField = std::vector<double>(aAnalysisDomainMeshView.size());
    for (const auto& tFieldProxy : aAnalysisDomainMeshView)
    {
        const auto& tField = static_cast<analysis::ScalarFieldValue>(tFieldProxy);
        tNodalField[tField.mDesignVariableVectorIndex] = tField.mValue;
    }
    return tNodalField;
}

}  // namespace

DesignVariablesConversion::DesignVariablesConversion(Mesh aMesh) : Mesh{std::move(aMesh)} {}

auto DesignVariablesConversion::nodalFieldToAnalysisDomainMesh(const NodalFieldVectorReference aScalarField) const
    -> analysis::AnalysisDomainMesh
{
    const auto tNodeIDs = [](const mesh::Mesh& aMesh, const Mesh::BlockOrdinalType aBlockOrdinal)
    { return mesh::MeshBlocks{aMesh}.nodeIDs(aBlockOrdinal); };
    return entity_field_to_analysis_domain_mesh(aScalarField.mValue.get(), *this, tNodeIDs);
}

auto DesignVariablesConversion::elementFieldToAnalysisDomainMesh(const ElementFieldVectorReference aScalarField) const
    -> analysis::AnalysisDomainMesh
{
    const auto tElementIDs = [](const mesh::Mesh& aMesh, const Mesh::BlockOrdinalType aBlockOrdinal)
    { return mesh::MeshBlocks{aMesh}.elementIDs(aBlockOrdinal); };
    return entity_field_to_analysis_domain_mesh(aScalarField.mValue.get(), *this, tElementIDs);
}

NodalFieldVector DesignVariablesConversion::analysisDomainMeshToNodalFieldVector(
    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    return NodalFieldVector{analysis_domain_mesh_to_vector(aAnalysisDomainMesh)};
}

ElementFieldVector DesignVariablesConversion::analysisDomainMeshToElementFieldVector(
    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    return ElementFieldVector{analysis_domain_mesh_to_vector(aAnalysisDomainMesh)};
}

}  // namespace plato::mesh
