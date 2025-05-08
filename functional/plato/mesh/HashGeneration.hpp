#ifndef PLATO_MESH_HASHGENERATION
#define PLATO_MESH_HASHGENERATION

#include <cstddef>

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::mesh
{
/// @brief Computes a hash of the nodal coordinates of the mesh referenced by the file name in
///  @a aAnalysisDomainMesh. Does not consider the density values or connectivity, only the nodal coordinates.
[[nodiscard]] auto hash_mesh_coordinates(const plato::analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> std::size_t;
}  // namespace plato::mesh

#endif
