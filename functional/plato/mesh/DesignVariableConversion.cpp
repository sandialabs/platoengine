#include "plato/mesh/DesignVariableConversion.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>

#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/MeshBlocks.hpp"
#include "plato/mesh/MeshDesignVariablesViews.hpp"
#include "plato/utilities/IndexRange.hpp"

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
        const auto tDensities = std::vector<double>(tIDs.size());
        const auto tBlockID = MeshBlocks{aMesh}.blockID(tDesignBlockOrdinal);
        assert(tBlockID.has_value());
        tMeshDesignVariables.mBlockDensities.emplace(tBlockID.value(),
                                                     detail::combine_densities_and_ids(tDensities, tIDs));
    }
    return tMeshDesignVariables;
}

template <typename F>
MeshDesignVariables entity_densities_to_mesh_design_variables(const std::vector<double>& aDensities,
                                                              const Mesh& aMesh,
                                                              const F& aIDFunction)
{
    auto tMeshDesignVariables = initialize_mesh_design_variable_data_structures(aMesh, aIDFunction);

    auto tMeshView = MeshDesignVariablesDensitiesMutableView{tMeshDesignVariables};
    assert(aDensities.size() == tMeshView.size());
    std::transform(aDensities.cbegin(), aDensities.cend(), tMeshView.begin(), tMeshView.begin(),
                   [tVectorIndex = Density::IndexType{0}](const double aDensity, const auto& aDensityID) mutable
                   {
                       const auto& tDensityIndices = static_cast<Density>(aDensityID);
                       return Density{tDensityIndices.mGlobalMeshEntityID, tVectorIndex++, aDensity};
                   });

    return tMeshDesignVariables;
}

std::vector<double> mesh_design_variables_view_to_vector(const MeshDesignVariables& tMeshDesignVariables)
{
    const auto aMeshDesignVariablesView = MeshDesignVariablesDensitiesView{tMeshDesignVariables};
    auto tNodalDensities = std::vector<double>(aMeshDesignVariablesView.size());
    for (const auto& tDensityProxy : aMeshDesignVariablesView)
    {
        const auto& tDensity = static_cast<Density>(tDensityProxy);
        tNodalDensities[tDensity.mDesignVariableVectorIndex] = tDensity.mDensity;
    }
    return tNodalDensities;
}
}  // namespace

DesignVariablesConversion::DesignVariablesConversion(Mesh aMesh) : Mesh{std::move(aMesh)} {}

MeshDesignVariables DesignVariablesConversion::nodalDensitiesToMeshDesignVariables(
    const NodalDensityVectorReference aDensities) const
{
    const auto tNodeIDs = [](const mesh::Mesh& aMesh, const Mesh::BlockOrdinalType aBlockOrdinal)
    { return mesh::MeshBlocks{aMesh}.nodeIDs(aBlockOrdinal); };
    return entity_densities_to_mesh_design_variables(aDensities.mValue.get(), *this, tNodeIDs);
}

MeshDesignVariables DesignVariablesConversion::elementDensitiesToMeshDesignVariables(
    const ElementDensityVectorReference aDensities) const
{
    const auto tNodeIDs = [](const mesh::Mesh& aMesh, const Mesh::BlockOrdinalType aBlockOrdinal)
    { return mesh::MeshBlocks{aMesh}.elementIDs(aBlockOrdinal); };
    return entity_densities_to_mesh_design_variables(aDensities.mValue.get(), *this, tNodeIDs);
}

NodalDensityVector DesignVariablesConversion::meshDesignVariablesToNodalDensityVector(
    const MeshDesignVariables& aMeshDesignVariables) const
{
    return NodalDensityVector{mesh_design_variables_view_to_vector(aMeshDesignVariables)};
}

ElementDensityVector DesignVariablesConversion::meshDesignVariablesToElementDensityVector(
    const MeshDesignVariables& aMeshDesignVariables) const
{
    return ElementDensityVector{mesh_design_variables_view_to_vector(aMeshDesignVariables)};
}

}  // namespace plato::mesh
