#include "plato/transformations/DistanceField.hpp"

#include <functional>
#include <ranges>

#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"

namespace plato::transformations
{
auto element_centroid_distance_field(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                     const Plane& aBuildPlane) -> analysis::AnalysisDomainMesh
{
    const auto tMesh = mesh::Mesh{aAnalysisDomainMesh.mFileName};
    const auto tElementCentroids = mesh::EntityRetrieval{tMesh}.elementCentroids();

    auto tCentroidToDistance =
        std::views::transform(tElementCentroids, [aBuildPlane](const auto& aCentroid)
                              { return detail::point_plane_signed_distance(aBuildPlane, aCentroid); });
    const auto tBuildPlaneDistances = std::vector(tCentroidToDistance.begin(), tCentroidToDistance.end());

    return mesh::DesignVariablesConversion{tMesh}.elementFieldToAnalysisDomainMesh(
        mesh::ElementFieldVectorReference{std::cref(tBuildPlaneDistances)});
}

}  // namespace plato::transformations
