#include <gtest/gtest.h>

#include <filesystem>
#include <iomanip>

#include "plato/utilities/TesselationTraits.hpp"

namespace plato::utilities::unittest
{

namespace
{

const std::vector<Coordinate> kQuadCoordinates{{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}};
const std::vector<Coordinate> kHexCoordinates{{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
                                              {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}};
const std::vector<Coordinate> kTetCoordinates{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
const std::vector<Coordinate> kTriCoordinates{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};

}  // namespace

TEST(TesselationTraits, Hex8Volume)
{
    {
        const auto tVolume = detail::volume_impl<stk::topology::HEXAHEDRON_8>(kHexCoordinates);
        ASSERT_DOUBLE_EQ(tVolume, 1.0);
    }
    {
        const auto tVolume = detail::volume_impl<stk::topology::HEX_8>(kHexCoordinates);
        ASSERT_DOUBLE_EQ(tVolume, 1.0);
    }
}

TEST(TesselationTraits, TetVolume)
{
    const double tGold =
        Tetrahedron{kTetCoordinates[0], kTetCoordinates[1], kTetCoordinates[2], kTetCoordinates[3]}.volume();

    {
        const auto tVolume = detail::volume_impl<stk::topology::TETRAHEDRON_4>(kTetCoordinates);
        ASSERT_DOUBLE_EQ(tVolume, tGold);
    }
    {
        const auto tVolume = detail::volume_impl<stk::topology::TET_4>(kTetCoordinates);
        ASSERT_DOUBLE_EQ(tVolume, tGold);
    }
}

TEST(TesselationTraits, Quad4Volume)
{
    {
        const auto tVolume = detail::volume_impl<stk::topology::QUADRILATERAL_4>(kQuadCoordinates);
        ASSERT_DOUBLE_EQ(tVolume, 1.0);
    }
    {
        const auto tVolume = detail::volume_impl<stk::topology::QUAD_4>(kQuadCoordinates);
        ASSERT_DOUBLE_EQ(tVolume, 1.0);
    }
}

TEST(TesselationTraits, Tri3Volume)
{
    {
        const auto tVolume = detail::volume_impl<stk::topology::TRIANGLE_3>(kTriCoordinates);
        ASSERT_DOUBLE_EQ(tVolume, 0.5);
    }
    {
        const auto tVolume = detail::volume_impl<stk::topology::TRI_3>(kTriCoordinates);
        ASSERT_DOUBLE_EQ(tVolume, 0.5);
    }
}

}  // namespace plato::utilities::unittest
