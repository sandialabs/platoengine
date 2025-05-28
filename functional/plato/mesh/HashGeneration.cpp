#include "plato/mesh/HashGeneration.hpp"

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/third_party_integration/common/Vector3Hash.hpp"
#include "plato/utilities/HashUtilities.hpp"

namespace plato::mesh
{
auto hash_mesh_coordinates(const plato::analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> std::size_t
{
    const auto tCoords = mesh::EntityRetrieval{mesh::Mesh{aAnalysisDomainMesh.mFileName}}.nodalCoordinates();
    return utilities::hash_container(tCoords);
}
}  // namespace plato::mesh
