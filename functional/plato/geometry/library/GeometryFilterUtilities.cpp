#include "plato/geometry/library/GeometryFilterUtilities.hpp"

#include "plato/core/Compose.hpp"
#include "plato/mesh/DesignVariableAdapter.hpp"
#include "plato/mesh/Mesh.hpp"

namespace plato::geometry::library
{
auto adapt_filter(const filter::library::FilterFunction& aFilterFunction, const mesh::Mesh& aMesh)
    -> AdaptedFilterFunction
{
    const auto tFilterFunction = core::compose(aFilterFunction, mesh::design_variables_to_analysis_mesh_adapter(aMesh));
    return core::compose(mesh::analysis_mesh_to_design_variables_adapter(aMesh), tFilterFunction);
}
}  // namespace plato::geometry::library
