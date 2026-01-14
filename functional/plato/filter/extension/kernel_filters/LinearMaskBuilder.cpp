#include "plato/filter/extension/kernel_filters/LinearMaskBuilder.hpp"

#include <vector>

#include "plato/filter/extension/kernel_filters/KernelFilterCenteringTypes.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"

namespace plato::filter::extension::kernel_filters
{
namespace detail
{
auto center_coordinates(const mesh::Mesh& aMesh, input_parser::KernelFilterCenteringTypes aCenteringType)
    -> std::vector<third_party_integration::common::Coordinate>
{
    if (aCenteringType == input_parser::KernelFilterCenteringTypes::kElementCentered)
    {
        return mesh::EntityRetrieval{aMesh}.designDomainElementCentroids();
    }
    else
    {
        return mesh::EntityRetrieval{aMesh}.designDomainNodalCoordinates();
    }
}
}  // namespace detail

}  // namespace plato::filter::extension::kernel_filters
