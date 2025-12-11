#include "plato/transformations/DistanceField.hpp"

#include <algorithm>
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
[[nodiscard]] auto vector_sum(const std::ranges::range auto& aLeft, std::vector<double>&& aRight) -> std::vector<double>
{
    std::transform(aRight.begin(), aRight.end(), aLeft.begin(), aRight.begin(),
                   [](const double aLeft, const double aRight) { return aLeft + aRight; });
    return std::move(aRight);
}

auto element_centroid_distance_field(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh, const Plane& aBuildPlane)
    -> analysis::AnalysisDomainMesh
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

auto row_vector_jacobian_multiplication_distance_field(const std::vector<double>&,
                                                       const analysis::AnalysisDomainMesh&,
                                                       const Plane&) -> std::vector<double>
{
    return {};
}

/// @brief Computes the multiplication of a row vector @a aRowVector with the adjoint Jacobian of the element centroid
/// distance field.
[[nodiscard]] auto row_vector_adjoint_jacobian_multiplication_distance_field(
    const std::vector<double>& aRowVector,
    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
    const Plane& aBuildPlane) -> std::vector<double>
{
    const auto tMesh = mesh::Mesh{aAnalysisDomainMesh.mFileName};
    const auto tMeshDimensions = mesh::EntityCounts{tMesh}.spatialDimensions();
    const auto tCoordinateView = utilities::MultiVectorView{aRowVector, tMeshDimensions};

    auto tResult = std::vector<double>(mesh::EntityCounts{tMesh}.numberOfElements());
    const auto tPlaneNormal = std::array{aBuildPlane.mNormal.x, aBuildPlane.mNormal.y, aBuildPlane.mNormal.z};
    for (const auto tDimension : std::views::iota(0U, tMeshDimensions))
    {
        const auto tDimensionAverage = mesh::MeshQuantities{tMesh}.nodalAverage(
            utilities::SingleDimensionMultiVectorView{tCoordinateView, tDimension});
        auto tAverageTimesNormalComponent =
            tDimensionAverage | std::views::transform([mComponent = tPlaneNormal[tDimension]](const double aValue)
                                                      { return mComponent * aValue; });
        tResult = vector_sum(tAverageTimesNormalComponent, std::move(tResult));
    }

    return tResult;
}

}  // namespace plato::transformations
