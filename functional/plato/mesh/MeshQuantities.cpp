#include "plato/mesh/MeshQuantities.hpp"

#include "plato/mesh/Mesh.hpp"

namespace plato::mesh
{
double average_nodal_density(const Mesh& aMesh)
{
    const auto tTotalNumberOfNodes = aMesh.numberOfNodes();
    const double tTotalVolume = aMesh.volume();
    return static_cast<double>(tTotalNumberOfNodes) / tTotalVolume;
}
}  // namespace plato::mesh
