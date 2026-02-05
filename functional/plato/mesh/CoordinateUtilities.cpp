#include "plato/mesh/CoordinateUtilities.hpp"

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/MultiVectorView.hpp"

namespace plato::mesh
{
auto dynamic_vector_to_nodal_coordinates(const linear_algebra::DynamicVector<double>& aCoordinateVector,
                                         const std::size_t aDimensions)
    -> std::vector<third_party_integration::common::Coordinate>
{
    const auto tVectorView = utilities::MultiVectorView{aCoordinateVector.stdVector(), aDimensions};
    const auto tNumberOfNodes = aCoordinateVector.size() / aDimensions;
    auto tCoordinates =
        utilities::reserved_container<std::vector<third_party_integration::common::Coordinate>>(tNumberOfNodes);

    std::ranges::transform(std::views::iota(0U, tNumberOfNodes), std::back_inserter(tCoordinates),
                           [tVectorView](const auto aIndex) -> third_party_integration::common::Coordinate
                           { return tVectorView(utilities::VectorIndex{aIndex}); });
    return tCoordinates;
}
}  // namespace plato::mesh
