#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <optional>

#include "plato/filter/extension/LinearMask.hpp"
#include "plato/filter/extension/parallel_unittest/LinearMaskTestUtility.hpp"
#include "plato/third_party_integration/common/unittest/CoordinateTestUtilities.hpp"

namespace plato::filter::extension::unittest
{

namespace
{

constexpr auto kNumRanks = int{4};

}  // namespace

TEST(LinearMaskDetail, MPISize)
{
    const auto tComm = boost::mpi::communicator{};
    EXPECT_EQ(tComm.size(), kNumRanks);
}

TEST(LinearMask, Apply)
{
    const LinearMask tLinearMask = create_simple_linear_mask<LinearMask>(std::nullopt);

    /* 2/3          1/3             0           0
       1/4         1/2            1/4        0
       0            1/4            1/2        1/4
       0            0               1/3         2/3*/

    const auto [tRows, tCols] = tLinearMask.size();
    ASSERT_EQ(tRows, 4);
    ASSERT_EQ(tCols, 4);

    {
        std::vector<double> tDensities{.5, 1, .2, .1};
        const auto tResult = tLinearMask.matrixMultiply(tDensities);

        ASSERT_EQ(tResult.size(), 4u);

        EXPECT_DOUBLE_EQ(tResult[0], 2.0 / 3.0);
        EXPECT_DOUBLE_EQ(tResult[1], 0.675);
        EXPECT_DOUBLE_EQ(tResult[2], 0.375);
        EXPECT_DOUBLE_EQ(tResult[3], 2.0 / 15.0);
    }
    {
        std::vector<double> tSensitivities{.1, .2, .3, .4};
        const auto tResult = tLinearMask.transposeMatrixMultiply(tSensitivities);

        ASSERT_EQ(tResult.size(), 4u);

        EXPECT_DOUBLE_EQ(tResult[0], 7.0 / 60.0);
        EXPECT_DOUBLE_EQ(tResult[1], 5.0 / 24.0);
        EXPECT_DOUBLE_EQ(tResult[2], 1.0 / 3.0);
        EXPECT_DOUBLE_EQ(tResult[3], 41.0 / 120.0);
    }
}

TEST(LinearMask, ApplyGivenCentroid)
{
    const std::vector<third_party_integration::common::Coordinate> tRelativeToCoordinate{{1, 0, 0}, {2, 0, 0}};
    const LinearMask tLinearMask = create_simple_linear_mask<LinearMask>(tRelativeToCoordinate);
    /*
           1/4         1/2            1/4        0
           0           1/4            1/2        1/4
    */
    const auto [tRows, tCols] = tLinearMask.size();
    ASSERT_EQ(tRows, 2);
    ASSERT_EQ(tCols, 4);

    {
        std::vector<double> tDensities{.5, 1, .2, .1};
        const auto tResult = tLinearMask.matrixMultiply(tDensities);

        ASSERT_EQ(tResult.size(), 2u);

        EXPECT_DOUBLE_EQ(tResult[0], 0.675);
        EXPECT_DOUBLE_EQ(tResult[1], 0.375);
    }
    {
        std::vector<double> tSensitivities{.5, .6};
        const auto tResult = tLinearMask.transposeMatrixMultiply(tSensitivities);

        ASSERT_EQ(tResult.size(), 4u);

        EXPECT_DOUBLE_EQ(tResult[0], 1.0 / 8.0);
        EXPECT_DOUBLE_EQ(tResult[1], 2.0 / 5.0);
        EXPECT_DOUBLE_EQ(tResult[2], 0.425);
        EXPECT_DOUBLE_EQ(tResult[3], 0.15);
    }
}

}  // namespace plato::filter::extension::unittest
