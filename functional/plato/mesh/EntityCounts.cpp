#include "plato/mesh/EntityCounts.hpp"

#include <string_view>
#include <utility>
#include <vector>

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"

namespace plato::mesh
{
EntityCounts::EntityCounts(Mesh aMeshBase) : Mesh{std::move(aMeshBase)} {}

auto EntityCounts::numberOfElements() const -> unsigned int
{
    return third_party_integration::stk_io::element_size(bulkData());
}

auto EntityCounts::numberOfDesignDomainElements() const -> unsigned int
{
    return third_party_integration::stk_io::element_size(bulkData(), designDomainBlocks());
}

auto EntityCounts::numberOfNodes() const -> unsigned int
{
    return third_party_integration::stk_io::node_size(bulkData());
}

auto EntityCounts::numberOfDesignDomainNodes() const -> unsigned int
{
    return third_party_integration::stk_io::node_size(bulkData(), designDomainBlocks());
}

auto EntityCounts::numberOfBlocks() const -> unsigned int
{
    return third_party_integration::stk_io::block_size(bulkData());
}

auto EntityCounts::spatialDimensions() const -> unsigned int
{
    return third_party_integration::stk_io::spatial_dimensions(bulkData());
}

auto EntityCounts::is2D() const -> bool { return spatialDimensions() == 2U; }

auto EntityCounts::is3D() const -> bool { return spatialDimensions() == 3U; }

auto EntityCounts::areNodalDesignVariables(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const -> bool
{
    const auto tAnalysisDomainMeshView = analysis::AnalysisDomainMeshSequentialView{aAnalysisDomainMesh};
    return numberOfDesignDomainNodes() == tAnalysisDomainMeshView.size();
}

auto EntityCounts::areElementDesignVariables(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const -> bool
{
    const auto tAnalysisDomainMeshView = analysis::AnalysisDomainMeshSequentialView{aAnalysisDomainMesh};
    return numberOfDesignDomainElements() == tAnalysisDomainMeshView.size();
}

auto EntityCounts::hasNodalFieldVariable(const std::string_view aFieldName) const -> bool
{
    return third_party_integration::stk_io::nodal_field_exists(filePath(), aFieldName);
}

auto EntityCounts::timeSteps() const -> std::vector<double>
{
    return third_party_integration::stk_io::time_steps(filePath());
}

auto EntityCounts::isAllTetAndTri() const -> bool
{
    return third_party_integration::stk_io::all_tet_or_tri_elements(bulkData());
}

}  // namespace plato::mesh
