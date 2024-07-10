#ifndef PLATO_MESH_MESHQUANTITIES
#define PLATO_MESH_MESHQUANTITIES

namespace plato::mesh
{
class Mesh;
}

namespace plato::mesh
{
/// @brief Returns the average nodal density by dividing the total nodes over the total volume of @a aMesh
[[nodiscard]] double average_nodal_density(const Mesh& aMesh);
}  // namespace plato::mesh

#endif
