#include <gtest/gtest.h>

#include "plato/filter/extension/LinearMask.hpp"
#include "plato/filter/extension/test_utilities/LinearMaskTestUtility.hpp"

namespace plato::filter::extension::unittest
{

TEST(LinearMask, SerialApply)
{
    const LinearMask tLinearMask = test_utilities::create_simple_linear_mask();
    const std::vector<double> tProjection{3, 4, 4, 3};
    /* 2/3          1/3             0           0
       1/4         1/2            1/4        0
       0            1/4            1/2        1/4
       0            0               1/3         2/3*/

    {
        const auto tResult = tLinearMask.matrixMultiply(tProjection);
        ASSERT_EQ(tResult.size(), 4u);
        EXPECT_DOUBLE_EQ(tResult[0], 10.0 / 3.0);
        EXPECT_DOUBLE_EQ(tResult[1], 3.75);
        EXPECT_DOUBLE_EQ(tResult[2], 3.75);
        EXPECT_DOUBLE_EQ(tResult[3], 10 / 3.);
    }
    {
        const auto tResult = tLinearMask.transposeMatrixMultiply(tProjection);
        ASSERT_EQ(tResult.size(), 4u);
        EXPECT_DOUBLE_EQ(tResult[0], 3);
        EXPECT_DOUBLE_EQ(tResult[1], 4);
        EXPECT_DOUBLE_EQ(tResult[2], 4);
        EXPECT_DOUBLE_EQ(tResult[3], 3);
    }
}

TEST(LinearMask, SerialApplyGivenCentroid)
{
    const std::vector<third_party_integration::common::Coordinate> tRelativeToCoordinate{{1, 0, 0}, {2, 0, 0}};
    const LinearMask tLinearMask = test_utilities::create_simple_linear_mask(tRelativeToCoordinate);
    /*
           1/4         1/2            1/4        0
           0           1/4            1/2        1/4
    */
    const auto [tRows, tCols] = tLinearMask.size();
    ASSERT_EQ(tRows, 2);
    ASSERT_EQ(tCols, 4);

    if (boost::mpi::communicator{}.rank() == 0)
    {
        const std::vector<double> tProjection{1, 0, 0, 0};
        const auto tResult = tLinearMask.matrixMultiply(tProjection);
        ASSERT_EQ(tResult.size(), 2u);
        EXPECT_DOUBLE_EQ(tResult[0], 0.25);
        EXPECT_DOUBLE_EQ(tResult[1], 0.);
    }

    {
        const std::vector<double> tDensities{.5, 1, .2, .1};
        const auto tResult = tLinearMask.matrixMultiply(tDensities);

        ASSERT_EQ(tResult.size(), 2u);

        EXPECT_DOUBLE_EQ(tResult[0], 0.675);
        EXPECT_DOUBLE_EQ(tResult[1], 0.375);
    }
    {
        const std::vector<double> tSensitivities{.5, .6};
        const auto tResult = tLinearMask.transposeMatrixMultiply(tSensitivities);

        ASSERT_EQ(tResult.size(), 4u);

        EXPECT_DOUBLE_EQ(tResult[0], 1.0 / 8.0);
        EXPECT_DOUBLE_EQ(tResult[1], 2.0 / 5.0);
        EXPECT_DOUBLE_EQ(tResult[2], 0.425);
        EXPECT_DOUBLE_EQ(tResult[3], 0.15);
    }
}

}  // namespace plato::filter::extension::unittest
