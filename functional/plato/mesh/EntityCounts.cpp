#include "plato/mesh/EntityCounts.hpp"

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"

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

bool EntityCounts::areNodalDesignVariables(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    const auto tAnalysisDomainMeshView = analysis::AnalysisDomainMeshSequentialView{aAnalysisDomainMesh};
    return numberOfDesignDomainNodes() == tAnalysisDomainMeshView.size();
}

bool EntityCounts::areElementDesignVariables(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    const auto tAnalysisDomainMeshView = analysis::AnalysisDomainMeshSequentialView{aAnalysisDomainMesh};
    return numberOfDesignDomainElements() == tAnalysisDomainMeshView.size();
}

bool EntityCounts::hasNodalFieldVariable(const std::string_view aFieldName) const
{
    return third_party_integration::stk_io::nodal_field_exists(filePath(), aFieldName);
}

auto EntityCounts::timeSteps() const -> std::vector<double>
{
    return third_party_integration::stk_io::time_steps(filePath());
}

}  // namespace plato::mesh
