#include <gtest/gtest.h>

#include <filesystem>
#include <iomanip>

#include "plato/third_party_integration/common/unittest/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/TesselationTraits.hpp"

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

TEST(TesselationTraits, Hex8Volume)
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

TEST(TesselationTraits, Hex8Centroid)
{
    const common::Coordinate tCentroid = detail::centroid_impl<stk::topology::HEXAHEDRON_8>(kHexCoordinates);
    const common::Coordinate tGold{0.5, 0.5, 0.5};
    common::unittest::test_double_equality_of_components(tCentroid, tGold,
                                                         TEST_CONTEXT("Tesselation traits Hex8 Centroid"));
}

TEST(TesselationTraits, TetVolume)
{
    const double tGold =
        Tetrahedron{kTetCoordinates[0], kTetCoordinates[1], kTetCoordinates[2], kTetCoordinates[3]}.volume();

    {
        const auto tVolume = detail::volume_impl<stk::topology::TETRAHEDRON_4>(kTetCoordinates);
        EXPECT_DOUBLE_EQ(tVolume, tGold);
    }
    {
        const auto tVolume = detail::volume_impl<stk::topology::TET_4>(kTetCoordinates);
        EXPECT_DOUBLE_EQ(tVolume, tGold);
    }
}

TEST(TesselationTraits, TetCentroid)
{
    const common::Coordinate tCentroid = detail::centroid_impl<stk::topology::TET_4>(kTetCoordinates);
    const common::Coordinate tGold{1.0 / 4.0, 1.0 / 4.0, 1.0 / 4.0};
    common::unittest::test_double_equality_of_components(tCentroid, tGold,
                                                         TEST_CONTEXT("Tesselation traits Tet Centroid"));
}

TEST(TesselationTraits, Quad4Volume)
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

TEST(TesselationTraits, QuadCentroid)
{
    const common::Coordinate tCentroid = detail::centroid_impl<stk::topology::QUADRILATERAL_4>(kQuadCoordinates);
    const common::Coordinate tGold{1.0 / 2.0, 1.0 / 2.0, 0.0};
    common::unittest::test_double_equality_of_components(tCentroid, tGold,
                                                         TEST_CONTEXT("Tesselation traits Quad Centroid"));
}

TEST(TesselationTraits, Tri3Volume)
{
    {
        const auto tVolume = detail::volume_impl<stk::topology::TRIANGLE_3>(kTriCoordinates);
        EXPECT_DOUBLE_EQ(tVolume, 0.5);
    }
    {
        const auto tVolume = detail::volume_impl<stk::topology::TRI_3>(kTriCoordinates);
        EXPECT_DOUBLE_EQ(tVolume, 0.5);
    }
}

TEST(TesselationTraits, TriCentroid)
{
    const common::Coordinate tCentroid = detail::centroid_impl<stk::topology::TRI_3>(kTriCoordinates);
    const common::Coordinate tGold{1.0 / 3.0, 1.0 / 3.0, 0.0};
    common::unittest::test_double_equality_of_components(tCentroid, tGold,
                                                         TEST_CONTEXT("Tesselation traits Tri Centroid"));
}

}  // namespace plato::third_party_integration::stk_io::unittest
