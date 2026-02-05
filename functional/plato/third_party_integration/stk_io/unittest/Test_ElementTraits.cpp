#include <gtest/gtest.h>

#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/ElementTraits.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/SensitivityTestUtilities.hpp"
#include "stk_topology/topology.hpp"

namespace plato::third_party_integration::stk_io::unittest
{

namespace
{

const std::vector<common::Coordinate> kQuadCoordinates{{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}};
const std::vector<common::Coordinate> kHexCoordinates{{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
                                                      {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}};
const std::vector<common::Coordinate> kTetCoordinates{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
const std::vector<common::Coordinate> kTriCoordinates{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};

}  // namespace

TEST(ElementTraits, Hex8Volume)
{
    {
        const auto tVolume = detail::volume_impl<stk::topology::HEXAHEDRON_8>(kHexCoordinates);
        EXPECT_DOUBLE_EQ(tVolume, 1.0);
    }
    {
        const auto tVolume = detail::volume_impl<stk::topology::HEX_8>(kHexCoordinates);
        EXPECT_DOUBLE_EQ(tVolume, 1.0);
    }
}

TEST(ElementTraits, Hex8Centroid)
{
    const common::Coordinate tCentroid = detail::centroid_impl<stk::topology::HEXAHEDRON_8>(kHexCoordinates);
    const common::Coordinate tGold{0.5, 0.5, 0.5};
    common::test_utilities::test_double_equality_of_components(tCentroid, tGold,
                                                               TEST_CONTEXT("Tesselation traits Hex8 Centroid"));
}

TEST(ElementTraits, HexVolumeNodalSensitivities)
{
    const auto tVolumeSensitivities = detail::volume_nodal_sensitivities_impl<stk::topology::HEX_8>(kHexCoordinates);
    const auto tExpectedSensitivities = test_utilities::axes_aligned_hex_volume_sensitivities(
        test_utilities::Cuboid{.mLength = 1.0, .mWidth = 1.0, .mHeight = 1.0});

    test_utilities::test_nodal_sensitivities_with_expected(tVolumeSensitivities, tExpectedSensitivities,
                                                           TEST_CONTEXT("Hexahedron volume nodal sensitivity"));
}

TEST(ElementTraits, TetVolume)
{
    constexpr double tGold = 1. / 6;
    {
        const auto tVolume = detail::volume_impl<stk::topology::TETRAHEDRON_4>(kTetCoordinates);
        EXPECT_DOUBLE_EQ(tVolume, tGold);
    }
    {
        const auto tVolume = detail::volume_impl<stk::topology::TET_4>(kTetCoordinates);
        EXPECT_DOUBLE_EQ(tVolume, tGold);
    }
}

TEST(ElementTraits, TetCentroid)
{
    const common::Coordinate tCentroid = detail::centroid_impl<stk::topology::TET_4>(kTetCoordinates);
    const common::Coordinate tGold{1.0 / 4.0, 1.0 / 4.0, 1.0 / 4.0};
    common::test_utilities::test_double_equality_of_components(tCentroid, tGold,
                                                               TEST_CONTEXT("Tesselation traits Tet Centroid"));
}

TEST(ElementTraits, TetVolumeNodalSensitivities)
{
    const auto tVolumeSensitivities = detail::volume_nodal_sensitivities_impl<stk::topology::TET_4>(kTetCoordinates);
    const auto tExpectedSensitivities = test_utilities::axes_aligned_tet_volume_sensitivities(
        test_utilities::Cuboid{.mLength = 1.0, .mWidth = 1.0, .mHeight = 1.0});

    test_utilities::test_nodal_sensitivities_with_expected(tVolumeSensitivities, tExpectedSensitivities,
                                                           TEST_CONTEXT("Tetrahedron volume nodal sensitivity"));
}

TEST(ElementTraits, Quad4Volume)
{
    {
        const auto tVolume = detail::volume_impl<stk::topology::QUADRILATERAL_4>(kQuadCoordinates);
        EXPECT_DOUBLE_EQ(tVolume, 1.0);
    }
    {
        const auto tVolume = detail::volume_impl<stk::topology::QUAD_4>(kQuadCoordinates);
        EXPECT_DOUBLE_EQ(tVolume, 1.0);
    }
}

TEST(ElementTraits, QuadCentroid)
{
    const common::Coordinate tCentroid = detail::centroid_impl<stk::topology::QUADRILATERAL_4>(kQuadCoordinates);
    const common::Coordinate tGold{1.0 / 2.0, 1.0 / 2.0, 0.0};
    common::test_utilities::test_double_equality_of_components(tCentroid, tGold,
                                                               TEST_CONTEXT("Tesselation traits Quad Centroid"));
}

TEST(ElementTraits, QuadVolumeNodalSensitivities)
{
    const auto tVolumeSensitivities = detail::volume_nodal_sensitivities_impl<stk::topology::QUAD_4>(kQuadCoordinates);
    const auto tExpectedSensitivities = test_utilities::axes_aligned_quad_volume_sensitivities(
        test_utilities::Rectangle{.mLength = 1.0, .mWidth = 1.0});

    test_utilities::test_nodal_sensitivities_with_expected(tVolumeSensitivities, tExpectedSensitivities,
                                                           TEST_CONTEXT("Quadrilateral volume nodal sensitivity"));
}

TEST(ElementTraits, Tri3Volume)
{
    constexpr double tGold{0.5};
    {
        const auto tVolume = detail::volume_impl<stk::topology::TRIANGLE_3>(kTriCoordinates);
        EXPECT_DOUBLE_EQ(tVolume, tGold);
    }
    {
        const auto tVolume = detail::volume_impl<stk::topology::TRI_3>(kTriCoordinates);
        EXPECT_DOUBLE_EQ(tVolume, tGold);
    }
}

TEST(ElementTraits, TriCentroid)
{
    const common::Coordinate tCentroid = detail::centroid_impl<stk::topology::TRI_3>(kTriCoordinates);
    const common::Coordinate tGold{1.0 / 3.0, 1.0 / 3.0, 0.0};
    common::test_utilities::test_double_equality_of_components(tCentroid, tGold,
                                                               TEST_CONTEXT("Tesselation traits Tri Centroid"));
}

TEST(ElementTraits, TriVolumeNodalSensitivities)
{
    const auto tVolumeSensitivities = detail::volume_nodal_sensitivities_impl<stk::topology::TRI_3>(kTriCoordinates);
    const auto tExpectedSensitivities =
        test_utilities::axes_aligned_tri_volume_sensitivities(test_utilities::Rectangle{.mLength = 1.0, .mWidth = 1.0});

    test_utilities::test_nodal_sensitivities_with_expected(tVolumeSensitivities, tExpectedSensitivities,
                                                           TEST_CONTEXT("Triangle volume nodal sensitivity"));
}

}  // namespace plato::third_party_integration::stk_io::unittest
