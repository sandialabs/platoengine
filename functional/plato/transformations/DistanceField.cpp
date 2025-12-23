#include "plato/transformations/DistanceField.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <ranges>

#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshQuantities.hpp"
#include "plato/utilities/MultiVectorView.hpp"
#include "plato/utilities/SingleDimensionMultiVectorView.hpp"

namespace plato::transformations
{
namespace
{
[[nodiscard]] auto normal_as_array(const Plane& aPlane) -> std::array<double, 3U>
{
    return std::array{aPlane.mNormal.x, aPlane.mNormal.y, aPlane.mNormal.z};
}
}  // namespace

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

auto row_vector_jacobian_multiplication_distance_field(const std::vector<double>& aRowVector,
                                                       const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                       const Plane& aBuildPlane) -> std::vector<double>
{
    const auto tMesh = mesh::Mesh{aAnalysisDomainMesh.mFileName};
    const auto tMeshDimensions = mesh::EntityCounts{tMesh}.spatialDimensions();
    const auto tNumberOfNodes = mesh::EntityCounts{tMesh}.numberOfNodes();

    const auto tPlaneNormal = normal_as_array(aBuildPlane);
    auto tFullNodalProjection = std::vector(static_cast<std::size_t>(tMeshDimensions * tNumberOfNodes), 0.0);
    const auto tRowVectorNodalProjection = mesh::MeshQuantities{tMesh}.nodalAverageElementProjection(aRowVector);
    for (const auto tDimension : std::views::iota(0U, tMeshDimensions))
    {
        const auto tCoordinateView = utilities::MultiVectorView{tFullNodalProjection, tMeshDimensions};
        const auto tDimensionView = utilities::SingleDimensionMultiVectorView{tCoordinateView, tDimension};
        auto tAverageTimesNormalComponent =
            tRowVectorNodalProjection | std::views::transform([mComponent = tPlaneNormal[tDimension]](
                                                                  const double aValue) { return mComponent * aValue; });
        std::ranges::copy(tAverageTimesNormalComponent, tDimensionView.begin());
    }
    return tFullNodalProjection;
}

auto row_vector_adjoint_jacobian_multiplication_distance_field(const std::vector<double>& aRowVector,
                                                               const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                               const Plane& aBuildPlane) -> std::vector<double>
{
    const auto tMesh = mesh::Mesh{aAnalysisDomainMesh.mFileName};
    const auto tMeshDimensions = mesh::EntityCounts{tMesh}.spatialDimensions();
    const auto tCoordinateView = utilities::MultiVectorView{aRowVector, tMeshDimensions};

    const auto tPlaneNormal = normal_as_array(aBuildPlane);
    auto tResult = std::vector<double>(mesh::EntityCounts{tMesh}.numberOfElements());
    for (const auto tDimension : std::views::iota(0U, tMeshDimensions))
    {
        const auto tDimensionAverage = mesh::MeshQuantities{tMesh}.elementAveragedNodalValues(
            utilities::SingleDimensionMultiVectorView{tCoordinateView, tDimension});
        auto tAverageTimesNormalComponent =
            tDimensionAverage | std::views::transform([mComponent = tPlaneNormal[tDimension]](const double aValue)
                                                      { return mComponent * aValue; });
        std::transform(tAverageTimesNormalComponent.begin(), tAverageTimesNormalComponent.end(), tResult.begin(),
                       tResult.begin(), std::plus<double>{});
    }

    return tResult;
}

}  // namespace plato::transformations
