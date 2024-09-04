#include "plato/mesh/DesignVariableConversion.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>

#include "plato/design_variables/MeshDesignVariablesSequentialView.hpp"
#include "plato/mesh/MeshBlocks.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::mesh
{
namespace
{
template <typename F>
design_variables::MeshDesignVariables initialize_mesh_design_variable_data_structures(const Mesh& aMesh,
                                                                                      const F& aIDFunction)
{
    auto tMeshDesignVariables = design_variables::MeshDesignVariables{aMesh.filePath(), {}};
    for (const auto tDesignBlockOrdinal : aMesh.designBlockOrdinals())
    {
        const auto tIDs = aIDFunction(aMesh, tDesignBlockOrdinal);
        const auto tField = std::vector<double>(tIDs.size());
        const auto tBlockID = MeshBlocks{aMesh}.blockID(tDesignBlockOrdinal);
        assert(tBlockID.has_value());
        tMeshDesignVariables.mBlockScalarField.emplace(
            tBlockID.value(), design_variables::combine_scalar_field_values_and_ids(tField, tIDs));
    }
    return tMeshDesignVariables;
}

template <typename F>
design_variables::MeshDesignVariables entity_field_to_mesh_design_variables(const std::vector<double>& aScalarField,
                                                                            const Mesh& aMesh,
                                                                            const F& aIDFunction)
{
    auto tMeshDesignVariables = initialize_mesh_design_variable_data_structures(aMesh, aIDFunction);

    auto tMeshView = design_variables::MeshDesignVariablesMutableSequentialView{tMeshDesignVariables};
    assert(aScalarField.size() == tMeshView.size());
    std::transform(
        aScalarField.cbegin(), aScalarField.cend(), tMeshView.begin(), tMeshView.begin(),
        [tVectorIndex = design_variables::ScalarFieldValue::IndexType{0}](const double aField,
                                                                          const auto& aFieldID) mutable
        {
            const auto& tFieldIndices = static_cast<design_variables::ScalarFieldValue>(aFieldID);
            return design_variables::ScalarFieldValue{tFieldIndices.mGlobalMeshEntityID, tVectorIndex++, aField};
        });

    return tMeshDesignVariables;
}

std::vector<double> mesh_design_variables_view_to_vector(
    const design_variables::MeshDesignVariables& tMeshDesignVariables)
{
    const auto aMeshDesignVariablesView = design_variables::MeshDesignVariablesSequentialView{tMeshDesignVariables};
    auto tNodalField = std::vector<double>(aMeshDesignVariablesView.size());
    for (const auto& tFieldProxy : aMeshDesignVariablesView)
    {
        const auto& tField = static_cast<design_variables::ScalarFieldValue>(tFieldProxy);
        tNodalField[tField.mDesignVariableVectorIndex] = tField.mValue;
    }
    return tNodalField;
}

auto mesh_design_variables_to_map(const design_variables::MeshDesignVariables& aMeshDesignVariables)
    -> std::unordered_map<design_variables::ScalarFieldValue::IndexType, double>
{
    auto tNodeIDFieldMap = std::unordered_map<design_variables::ScalarFieldValue::IndexType, double>{};
    const auto tDesignVariablesView = design_variables::MeshDesignVariablesSequentialView{aMeshDesignVariables};
    tNodeIDFieldMap.reserve(tDesignVariablesView.size());
    std::transform(tDesignVariablesView.begin(), tDesignVariablesView.end(),
                   std::inserter(tNodeIDFieldMap, tNodeIDFieldMap.begin()),
                   [](const auto& tProxy)
                   {
                       const auto tValue = static_cast<design_variables::ScalarFieldValue>(tProxy);
                       return std::make_pair(tValue.mGlobalMeshEntityID, tValue.mValue);
                   });
    return tNodeIDFieldMap;
}

}  // namespace

DesignVariablesConversion::DesignVariablesConversion(Mesh aMesh) : Mesh{std::move(aMesh)} {}

design_variables::MeshDesignVariables DesignVariablesConversion::nodalFieldToMeshDesignVariables(
    const NodalFieldVectorReference aScalarField) const
{
    const auto tNodeIDs = [](const mesh::Mesh& aMesh, const Mesh::BlockOrdinalType aBlockOrdinal)
    { return mesh::MeshBlocks{aMesh}.nodeIDs(aBlockOrdinal); };
    return entity_field_to_mesh_design_variables(aScalarField.mValue.get(), *this, tNodeIDs);
}

design_variables::MeshDesignVariables DesignVariablesConversion::elementFieldToMeshDesignVariables(
    const ElementFieldVectorReference aScalarField) const
{
    const auto tElementIDs = [](const mesh::Mesh& aMesh, const Mesh::BlockOrdinalType aBlockOrdinal)
    { return mesh::MeshBlocks{aMesh}.elementIDs(aBlockOrdinal); };
    return entity_field_to_mesh_design_variables(aScalarField.mValue.get(), *this, tElementIDs);
}

NodalFieldVector DesignVariablesConversion::meshDesignVariablesToNodalFieldVector(
    const design_variables::MeshDesignVariables& aMeshDesignVariables) const
{
    return NodalFieldVector{mesh_design_variables_view_to_vector(aMeshDesignVariables)};
}

ElementFieldVector DesignVariablesConversion::meshDesignVariablesToElementFieldVector(
    const design_variables::MeshDesignVariables& aMeshDesignVariables) const
{
    return ElementFieldVector{mesh_design_variables_view_to_vector(aMeshDesignVariables)};
}

auto DesignVariablesConversion::nodalFieldToNodalIDMap(const NodalFieldVectorReference aScalarField) const
    -> std::unordered_map<design_variables::ScalarFieldValue::IndexType, double>
{
    return mesh_design_variables_to_map(nodalFieldToMeshDesignVariables(aScalarField));
}

auto DesignVariablesConversion::elementFieldToElementIDMap(ElementFieldVectorReference aElementField) const
    -> std::unordered_map<design_variables::ScalarFieldValue::IndexType, double>
{
    return mesh_design_variables_to_map(elementFieldToMeshDesignVariables(aElementField));
}

auto DesignVariablesConversion::meshDesignVariablesToIDMap(
    const design_variables::MeshDesignVariables& aMeshDesignVariables) const
    -> std::unordered_map<design_variables::ScalarFieldValue::IndexType, double>
{
    return mesh_design_variables_to_map(aMeshDesignVariables);
}
}  // namespace plato::mesh
