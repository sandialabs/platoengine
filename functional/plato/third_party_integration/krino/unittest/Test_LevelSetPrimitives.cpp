#include <gtest/gtest.h>

#include <Akri_AnalyticSurf.hpp>
#include <Akri_BoundingBox.hpp>
#include <Akri_Composite_Surface.hpp>
#include <vector>

#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"

namespace plato::third_party_integration::krino::unittest
{
namespace
{
using third_party_integration::krino::test_utilities::KrinoTestFixture;
const auto kSphereOne = Sphere{{1, 2, 3}, 4};
const auto kBoundingBoxOne =
    ::krino::BoundingBox{std::array<double, 3>{-3.0, -2, -1}, std::array<double, 3>{5.0, 6, 7}};
const auto kSphereTwo = Sphere{{-5, -6, -7}, 3};
const auto kBoundingBoxTwo =
    ::krino::BoundingBox{std::array<double, 3>{-8.0, -9, -10}, std::array<double, 3>{-2.0, -3, -4}};

const auto kPositiveX = Plane{{1, 0, 0}, 0};

}  // namespace

TEST_F(KrinoTestFixture, AppendSpheresEmptySpheres)
{
    auto tSurfaces = make_krino_composite_surface();
    const auto tSpheres = std::vector<Sphere>{};

    EXPECT_EQ(tSurfaces.size(), 0U);
    append_spheres(tSurfaces, tSpheres);
    EXPECT_EQ(tSurfaces.size(), 0U);
}

TEST_F(KrinoTestFixture, AppendOneSphereAtATime)
{
    auto tSurfaces = make_krino_composite_surface();
    const auto tSpheresOne = std::vector<Sphere>{kSphereOne};
    const auto tSpheresTwo = std::vector<Sphere>{kSphereTwo};

    EXPECT_EQ(tSurfaces.size(), 0U);
    append_spheres(tSurfaces, tSpheresOne);
    EXPECT_EQ(tSurfaces.size(), 1U);

    EXPECT_TRUE(tSurfaces.does_intersect(kBoundingBoxOne));
    EXPECT_FALSE(tSurfaces.does_intersect(kBoundingBoxTwo));

    append_spheres(tSurfaces, tSpheresTwo);
    EXPECT_EQ(tSurfaces.size(), 2U);
    EXPECT_TRUE(tSurfaces.does_intersect(kBoundingBoxOne));
    EXPECT_TRUE(tSurfaces.does_intersect(kBoundingBoxTwo));
}

TEST_F(KrinoTestFixture, AppendSpheres)
{
    auto tSurfaces = make_krino_composite_surface();
    const auto tSpheres = std::vector<Sphere>{kSphereOne, kSphereTwo};

    EXPECT_EQ(tSurfaces.size(), 0U);
    append_spheres(tSurfaces, tSpheres);
    ASSERT_EQ(tSurfaces.size(), 2U);

    EXPECT_TRUE(tSurfaces.does_intersect(kBoundingBoxOne));
    EXPECT_TRUE(tSurfaces.does_intersect(kBoundingBoxTwo));
}

TEST_F(KrinoTestFixture, AppendPlanesEmpty)
{
    auto tSurfaces = make_krino_composite_surface();
    const auto tPlanes = std::vector<Plane>{};

    EXPECT_EQ(tSurfaces.size(), 0U);
    append_planes(tSurfaces, tPlanes);
    EXPECT_EQ(tSurfaces.size(), 0U);
}

TEST_F(KrinoTestFixture, AppendPlanes)
{
    auto tSurfaces = make_krino_composite_surface();
    const auto tPlanes = std::vector<Plane>{kPositiveX};

    EXPECT_EQ(tSurfaces.size(), 0U);
    append_planes(tSurfaces, tPlanes);
    EXPECT_EQ(tSurfaces.size(), 1U);
}

}  // namespace plato::third_party_integration::krino::unittest
