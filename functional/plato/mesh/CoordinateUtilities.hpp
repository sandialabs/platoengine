#ifndef PLATO_MESH_COORDINATEUTILITIES
#define PLATO_MESH_COORDINATEUTILITIES

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/utilities/Enumerate.hpp"
#include "plato/utilities/MultiVectorView.hpp"

namespace plato::mesh
{
/// @brief Converts a nodal vector field stored in a std::vector of Container3 objects @a aNodalField to a DynamicVector
template <third_party_integration::common::Container3Type Container3>
auto nodal_vector_field_to_dynamic_vector(const std::vector<Container3>& aNodalField, const std::size_t aDimensions)
    -> linear_algebra::DynamicVector<double>;

/// @brief Converts a DynamicVector @a aCoordinateVector containing nodal coordinates to a std::vector of Coordinate
/// objects.
/// @pre The coordinates in @a aCoordinate vector are stored in order of their x,y,z coordinates for each node, i.e.
/// [x0,y0,z0,x1,y1,z1,...xN,yN,zN]
auto dynamic_vector_to_nodal_coordinates(const linear_algebra::DynamicVector<double>& aCoordinateVector,
                                         const std::size_t aDimensions)
    -> std::vector<third_party_integration::common::Coordinate>;

template <third_party_integration::common::Container3Type Container3>
auto nodal_vector_field_to_dynamic_vector(const std::vector<Container3>& aNodalField, const std::size_t aDimensions)
    -> linear_algebra::DynamicVector<double>
{
    std::vector<double> tFieldVector(aDimensions * aNodalField.size());
    auto tFieldView = utilities::MultiVectorView(tFieldVector, aDimensions);
    for (const auto& [tIndex, tValue] : utilities::enumerate(aNodalField))
    {
        tFieldView(utilities::VectorIndex{tIndex}) = tValue;
    }
    return linear_algebra::DynamicVector(std::move(tFieldVector));
}
}  // namespace plato::mesh

#endif
