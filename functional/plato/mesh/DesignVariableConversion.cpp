#include "plato/mesh/DesignVariableConversion.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>

#include "plato/mesh/MeshBlocks.hpp"
#include "plato/mesh/MeshDesignVariablesSequentialView.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::mesh
{
namespace
{
template <typename F>
MeshDesignVariables initialize_mesh_design_variable_data_structures(const Mesh& aMesh, const F& aIDFunction)
{
    auto tMeshDesignVariables = MeshDesignVariables{aMesh.filePath(), {}};
    for (const auto tDesignBlockOrdinal : aMesh.designBlockOrdinals())
    {
        const auto tIDs = aIDFunction(aMesh, tDesignBlockOrdinal);
        const auto tField = std::vector<double>(tIDs.size());
        const auto tBlockID = MeshBlocks{aMesh}.blockID(tDesignBlockOrdinal);
        assert(tBlockID.has_value());
        tMeshDesignVariables.mBlockScalarField.emplace(tBlockID.value(),
                                                       detail::combine_scalar_field_values_and_ids(tField, tIDs));
    }
    return tMeshDesignVariables;
}

template <typename F>
MeshDesignVariables entity_field_to_mesh_design_variables(const std::vector<double>& aScalarField,
                                                          const Mesh& aMesh,
                                                          const F& aIDFunction)
{
    auto tMeshDesignVariables = initialize_mesh_design_variable_data_structures(aMesh, aIDFunction);

    auto tMeshView = MeshDesignVariablesMutableSequentialView{tMeshDesignVariables};
    assert(aScalarField.size() == tMeshView.size());
    std::transform(aScalarField.cbegin(), aScalarField.cend(), tMeshView.begin(), tMeshView.begin(),
                   [tVectorIndex = ScalarFieldValue::IndexType{0}](const double aField, const auto& aFieldID) mutable
                   {
                       const auto& tFieldIndices = static_cast<ScalarFieldValue>(aFieldID);
                       return ScalarFieldValue{tFieldIndices.mGlobalMeshEntityID, tVectorIndex++, aField};
                   });

    return tMeshDesignVariables;
}

std::vector<double> mesh_design_variables_view_to_vector(const MeshDesignVariables& tMeshDesignVariables)
{
    const auto aMeshDesignVariablesView = MeshDesignVariablesSequentialView{tMeshDesignVariables};
    auto tNodalField = std::vector<double>(aMeshDesignVariablesView.size());
    for (const auto& tFieldProxy : aMeshDesignVariablesView)
    {
        const auto& tField = static_cast<ScalarFieldValue>(tFieldProxy);
        tNodalField[tField.mDesignVariableVectorIndex] = tField.mValue;
    }
    return tNodalField;
}

auto mesh_design_variables_to_map(const MeshDesignVariables& aMeshDesignVariables)
    -> std::unordered_map<ScalarFieldValue::IndexType, double>
{
    auto tNodeIDFieldMap = std::unordered_map<ScalarFieldValue::IndexType, double>{};
    const auto tDesignVariablesView = MeshDesignVariablesSequentialView{aMeshDesignVariables};
    tNodeIDFieldMap.reserve(tDesignVariablesView.size());
    std::transform(tDesignVariablesView.begin(), tDesignVariablesView.end(),
                   std::inserter(tNodeIDFieldMap, tNodeIDFieldMap.begin()),
                   [](const auto& tProxy)
                   {
                       const auto tValue = static_cast<ScalarFieldValue>(tProxy);
                       return std::make_pair(tValue.mGlobalMeshEntityID, tValue.mValue);
                   });
    return tNodeIDFieldMap;
}

}  // namespace

DesignVariablesConversion::DesignVariablesConversion(Mesh aMesh) : Mesh{std::move(aMesh)} {}

MeshDesignVariables DesignVariablesConversion::nodalFieldToMeshDesignVariables(
    const NodalFieldVectorReference aScalarField) const
{
    const auto tNodeIDs = [](const mesh::Mesh& aMesh, const Mesh::BlockOrdinalType aBlockOrdinal)
    { return mesh::MeshBlocks{aMesh}.nodeIDs(aBlockOrdinal); };
    return entity_field_to_mesh_design_variables(aScalarField.mValue.get(), *this, tNodeIDs);
}

MeshDesignVariables DesignVariablesConversion::elementFieldToMeshDesignVariables(
    const ElementFieldVectorReference aScalarField) const
{
    const auto tElementIDs = [](const mesh::Mesh& aMesh, const Mesh::BlockOrdinalType aBlockOrdinal)
    { return mesh::MeshBlocks{aMesh}.elementIDs(aBlockOrdinal); };
    return entity_field_to_mesh_design_variables(aScalarField.mValue.get(), *this, tElementIDs);
}

NodalFieldVector DesignVariablesConversion::meshDesignVariablesToNodalFieldVector(
    const MeshDesignVariables& aMeshDesignVariables) const
{
    return NodalFieldVector{mesh_design_variables_view_to_vector(aMeshDesignVariables)};
}

ElementFieldVector DesignVariablesConversion::meshDesignVariablesToElementFieldVector(
    const MeshDesignVariables& aMeshDesignVariables) const
{
    return ElementFieldVector{mesh_design_variables_view_to_vector(aMeshDesignVariables)};
}

auto DesignVariablesConversion::nodalFieldToNodalIDMap(const NodalFieldVectorReference aScalarField) const
    -> std::unordered_map<ScalarFieldValue::IndexType, double>
{
    return mesh_design_variables_to_map(nodalFieldToMeshDesignVariables(aScalarField));
}

auto DesignVariablesConversion::elementFieldToElementIDMap(ElementFieldVectorReference aElementField) const
    -> std::unordered_map<ScalarFieldValue::IndexType, double>
{
    return mesh_design_variables_to_map(elementFieldToMeshDesignVariables(aElementField));
}

auto DesignVariablesConversion::meshDesignVariablesToIDMap(const MeshDesignVariables& aMeshDesignVariables) const
    -> std::unordered_map<ScalarFieldValue::IndexType, double>
{
    return mesh_design_variables_to_map(aMeshDesignVariables);
}
}  // namespace plato::mesh
