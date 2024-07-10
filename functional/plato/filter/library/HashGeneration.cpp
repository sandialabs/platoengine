#include "plato/filter/library/HashGeneration.hpp"

#include "plato/mesh/MeshProxy.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::filter::library
{
std::size_t hash_mesh(const plato::mesh::MeshProxy& aMeshProxy)
{
    namespace stk_io = plato::third_party_integration::stk_io;
    const auto tBulk = stk_io::read_mesh_bulk_data(aMeshProxy.mFileName);
    const auto tCoords = stk_io::flattened_nodal_coordinates(*tBulk);
    return detail::hash_container(tCoords);
}

}  // namespace plato::filter::library