#include <gtest/gtest.h>

#include <ranges>

#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/Containers.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/transformations/DistanceField.hpp"
#include "plato/utilities/ContainerHelpers.hpp"

namespace plato::transformations::unittest
{
namespace
{
namespace tpic = third_party_integration::common;

constexpr auto kTestPoint = tpic::Coordinate{.x = 10.0, .y = 11.0, .z = 12.0};

using DistanceFieldHexMeshTest = third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;
using DistanceField2DMeshTest = third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;

}  // namespace

TEST_F(DistanceFieldHexMeshTest, ElementCentroidDistanceField)
{
    constexpr auto tBuildPlane = Plane{.mOriginSignedDistance = 0.0, .mNormal = {.x = 0.0, .y = 0.0, .z = 1.0}};
    const auto tMeshWithDistanceField = element_centroid_distance_field(
        analysis::AnalysisDomainMesh{.mFileName = mMeshFilePath, .mBlockScalarField = {}}, tBuildPlane);

    ASSERT_EQ(tMeshWithDistanceField.mBlockScalarField.size(), 1U);
    ASSERT_EQ(tMeshWithDistanceField.mBlockScalarField.at(1U).size(), 3U);

    EXPECT_DOUBLE_EQ(tMeshWithDistanceField.mBlockScalarField.at(1U).at(0U).mValue, 0.5);
    EXPECT_DOUBLE_EQ(tMeshWithDistanceField.mBlockScalarField.at(1U).at(1U).mValue, 0.5);
    EXPECT_DOUBLE_EQ(tMeshWithDistanceField.mBlockScalarField.at(1U).at(2U).mValue, 0.5);
}

TEST_F(DistanceField2DMeshTest, ElementCentroidDistanceField)
{
    constexpr auto tBuildPlane = Plane{.mOriginSignedDistance = 2.0, .mNormal = {.x = -1.0, .y = 0.0, .z = 0.0}};
    const auto tMeshWithDistanceField = element_centroid_distance_field(
        analysis::AnalysisDomainMesh{.mFileName = mMeshFilePath, .mBlockScalarField = {}}, tBuildPlane);

    ASSERT_EQ(tMeshWithDistanceField.mBlockScalarField.size(), 3U);
    ASSERT_EQ(tMeshWithDistanceField.mBlockScalarField.at(1U).size(), 4U);
    ASSERT_EQ(tMeshWithDistanceField.mBlockScalarField.at(2U).size(), 2U);
    ASSERT_EQ(tMeshWithDistanceField.mBlockScalarField.at(3U).size(), 1U);

    // Expected values computed via matlab
    EXPECT_DOUBLE_EQ(tMeshWithDistanceField.mBlockScalarField.at(1U).at(0U).mValue, 5.0 / 3.0);
    EXPECT_DOUBLE_EQ(tMeshWithDistanceField.mBlockScalarField.at(1U).at(1U).mValue, 1.0);
    EXPECT_DOUBLE_EQ(tMeshWithDistanceField.mBlockScalarField.at(1U).at(2U).mValue, 1.0 / 3.0);
    EXPECT_DOUBLE_EQ(tMeshWithDistanceField.mBlockScalarField.at(1U).at(3U).mValue, 1.0);

    EXPECT_DOUBLE_EQ(tMeshWithDistanceField.mBlockScalarField.at(2U).at(0U).mValue, 8.0 / 3.0);
    EXPECT_DOUBLE_EQ(tMeshWithDistanceField.mBlockScalarField.at(2U).at(1U).mValue, 10.0 / 3.0);

    EXPECT_DOUBLE_EQ(tMeshWithDistanceField.mBlockScalarField.at(3U).at(0U).mValue, 1.0);
}

TEST_F(DistanceFieldHexMeshTest, RowVectorJacobianMultiplicationDistanceField) {}

TEST_F(DistanceFieldHexMeshTest, RowVectorAdjointJacobianMultiplicationDistanceField)
{
    // Since this is an affine transformation, the row-vector-adjoint Jacobian multiplication is the same as the
    // function evaluation, after subtracting the offset.

    // Zero offset
    for (const auto tOffset : {0.0, 1.0, -1.0})
    {
        const auto tBuildPlane = Plane{.mOriginSignedDistance = tOffset, .mNormal = {.x = 0.0, .y = 1.0, .z = 0.0}};
        const auto tMeshWithDistanceField = element_centroid_distance_field(
            analysis::AnalysisDomainMesh{.mFileName = mMeshFilePath, .mBlockScalarField = {}}, tBuildPlane);

        const auto tNodalCoordinates = mesh::EntityRetrieval{mesh::Mesh{mMeshFilePath}}.nodalCoordinates();

        auto tFlattenedCoordinatesView =
            tNodalCoordinates |
            std::views::transform([](const auto& aCoordinate)
                                  { return std::array{aCoordinate.x, aCoordinate.y, aCoordinate.z}; }) |
            std::views::join | std::views::common;

        const auto tFlattenedCoordinates =
            std::vector(tFlattenedCoordinatesView.begin(), tFlattenedCoordinatesView.end());

        const auto tVectorAdjointJacobianResult = row_vector_adjoint_jacobian_multiplication_distance_field(
            tFlattenedCoordinates, tMeshWithDistanceField, tBuildPlane);

        ASSERT_EQ(tMeshWithDistanceField.mBlockScalarField.size(), 1U);
        auto tToDistanceField = tMeshWithDistanceField.mBlockScalarField.at(1U) |
                                std::views::transform([tOffset](const auto& aScalarFieldValue)
                                                      { return aScalarFieldValue.mValue - tOffset; });
        const auto tDistanceField = std::vector(tToDistanceField.begin(), tToDistanceField.end());

        constexpr auto tAbsoluteTolerance = 1e-15;
        test_utilities::expect_container_entries_near(tVectorAdjointJacobianResult, tDistanceField, tAbsoluteTolerance,
                                                      TEST_CONTEXT("Zero offset"));
    }
}

TEST(DistanceField, PointPlaneDistanceZeroOffsetCartesianNormals)
{
    const auto tXPlane = Plane{.mOriginSignedDistance = 0.0, .mNormal = {.x = 1.0, .y = 0.0, .z = 0.0}};
    const auto tYPlane = Plane{.mOriginSignedDistance = 0.0, .mNormal = {.x = 0.0, .y = 1.0, .z = 0.0}};
    const auto tZPlane = Plane{.mOriginSignedDistance = 0.0, .mNormal = {.x = 0.0, .y = 0.0, .z = 1.0}};
    {
        EXPECT_EQ(detail::point_plane_signed_distance(tXPlane, kTestPoint), kTestPoint.x);
        EXPECT_EQ(detail::point_plane_signed_distance(tYPlane, kTestPoint), kTestPoint.y);
        EXPECT_EQ(detail::point_plane_signed_distance(tZPlane, kTestPoint), kTestPoint.z);
    }
    {
        constexpr auto tTestPointBelow = tpic::Coordinate{.x = -10.0, .y = -11.0, .z = -12.0};
        EXPECT_EQ(detail::point_plane_signed_distance(tXPlane, tTestPointBelow), tTestPointBelow.x);
        EXPECT_EQ(detail::point_plane_signed_distance(tYPlane, tTestPointBelow), tTestPointBelow.y);
        EXPECT_EQ(detail::point_plane_signed_distance(tZPlane, tTestPointBelow), tTestPointBelow.z);
    }
}

TEST(DistanceField, PointPlaneDistanceNonZeroOffsetCartesianNormals)
{
    constexpr auto tOriginDistance = -5.0;
    constexpr auto tXPlane = Plane{.mOriginSignedDistance = tOriginDistance, .mNormal = {.x = 1.0, .y = 0.0, .z = 0.0}};
    constexpr auto tYPlane = Plane{.mOriginSignedDistance = tOriginDistance, .mNormal = {.x = 0.0, .y = 1.0, .z = 0.0}};
    constexpr auto tZPlane = Plane{.mOriginSignedDistance = tOriginDistance, .mNormal = {.x = 0.0, .y = 0.0, .z = 1.0}};

    {
        EXPECT_EQ(detail::point_plane_signed_distance(tXPlane, kTestPoint), kTestPoint.x + tOriginDistance);
        EXPECT_EQ(detail::point_plane_signed_distance(tYPlane, kTestPoint), kTestPoint.y + tOriginDistance);
        EXPECT_EQ(detail::point_plane_signed_distance(tZPlane, kTestPoint), kTestPoint.z + tOriginDistance);

        // Point on plane
        constexpr auto tPointOnXPlane = tpic::Coordinate{.x = 5.0, .y = 10.0, .z = -3.0};
        EXPECT_EQ(detail::point_plane_signed_distance(tXPlane, tPointOnXPlane), 0.0);
    }
    {
        constexpr auto tTestPointBelow = tpic::Coordinate{.x = -10.0, .y = -11.0, .z = -12.0};
        EXPECT_EQ(detail::point_plane_signed_distance(tXPlane, tTestPointBelow), tTestPointBelow.x + tOriginDistance);
        EXPECT_EQ(detail::point_plane_signed_distance(tYPlane, tTestPointBelow), tTestPointBelow.y + tOriginDistance);
        EXPECT_EQ(detail::point_plane_signed_distance(tZPlane, tTestPointBelow), tTestPointBelow.z + tOriginDistance);

        // Point on plane
        constexpr auto tPointOnYPlane = tpic::Coordinate{.x = 15.0, .y = 5.0, .z = -3.0};
        EXPECT_EQ(detail::point_plane_signed_distance(tYPlane, tPointOnYPlane), 0.0);
    }
}

TEST(DistanceField, PointPlaneDistanceArbitrary)
{
    constexpr auto tOriginDistance = 30.0;
    constexpr auto tNormal = tpic::Vector3{.x = 3.0, .y = 0.0, .z = -4.0};
    constexpr auto tNormalMagnitude = 5.0;
    constexpr auto tPlane = Plane{.mOriginSignedDistance = tOriginDistance, .mNormal = tNormal};
    constexpr auto tOrigin = tpic::Coordinate{.x = 0.0, .y = 0.0, .z = 0.0};
    constexpr auto tExpected = tpic::dot(tNormal, kTestPoint - tOrigin) / tNormalMagnitude + tOriginDistance;
    EXPECT_DOUBLE_EQ(detail::point_plane_signed_distance(tPlane, kTestPoint), tExpected);

    // Distance to origin
    EXPECT_DOUBLE_EQ(detail::point_plane_signed_distance(tPlane, tOrigin), tOriginDistance);

    // Point on plane
    constexpr auto tPointOnPlane = tpic::Coordinate{0.0, 0.0, -tOriginDistance * tNormalMagnitude / tNormal.z};
    EXPECT_DOUBLE_EQ(detail::point_plane_signed_distance(tPlane, tPointOnPlane), 0.0);
}
}  // namespace plato::transformations::unittest
