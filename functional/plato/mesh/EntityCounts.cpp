#include "plato/mesh/EntityCounts.hpp"

#include "plato/design_variables/MeshDesignVariablesSequentialView.hpp"
#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::mesh
{
EntityCounts::EntityCounts(Mesh aMeshBase) : Mesh{std::move(aMeshBase)} {}

unsigned int EntityCounts::numberOfElements() const
{
    return third_party_integration::stk_io::element_size(bulkData());
}

unsigned int EntityCounts::numberOfDesignDomainElements() const
{
    return third_party_integration::stk_io::element_size(bulkData(), designDomainBlocks());
}

unsigned int EntityCounts::numberOfNodes() const { return third_party_integration::stk_io::node_size(bulkData()); }

unsigned int EntityCounts::numberOfDesignDomainNodes() const
{
    return third_party_integration::stk_io::node_size(bulkData(), designDomainBlocks());
}

unsigned int EntityCounts::numberOfBlocks() const { return third_party_integration::stk_io::block_size(bulkData()); }

unsigned int EntityCounts::spatialDimensions() const
{
    return third_party_integration::stk_io::spatial_dimensions(bulkData());
}

bool EntityCounts::is2D() const { return spatialDimensions() == 2u; }

bool EntityCounts::is3D() const { return spatialDimensions() == 3u; }

bool EntityCounts::areNodalDesignVariables(const design_variables::MeshDesignVariables& aMeshDesignVariables) const
{
    const auto tMeshDesignVariablesView = design_variables::MeshDesignVariablesSequentialView{aMeshDesignVariables};
    return numberOfDesignDomainNodes() == tMeshDesignVariablesView.size();
}

bool EntityCounts::areElementDesignVariables(const design_variables::MeshDesignVariables& aMeshDesignVariables) const
{
    const auto tMeshDesignVariablesView = design_variables::MeshDesignVariablesSequentialView{aMeshDesignVariables};
    return numberOfDesignDomainElements() == tMeshDesignVariablesView.size();
}

}  // namespace plato::mesh
