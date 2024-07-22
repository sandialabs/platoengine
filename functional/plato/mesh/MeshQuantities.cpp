#include "plato/mesh/MeshQuantities.hpp"

#include "plato/third_party_integration/stk_io/Utilities.hpp"
#include "plato/third_party_integration/stk_io/VolumeUtilities.hpp"

namespace plato::mesh
{
MeshQuantities::MeshQuantities(Mesh aMeshBase) : Mesh{std::move(aMeshBase)} {}

double MeshQuantities::volume() const { return third_party_integration::stk_io::mesh_volume(bulkData()); }

double MeshQuantities::averageNodalDensity() const
{
    const auto tTotalNumberOfNodes = third_party_integration::stk_io::node_size(bulkData());
    return static_cast<double>(tTotalNumberOfNodes) / volume();
}

}  // namespace plato::mesh
