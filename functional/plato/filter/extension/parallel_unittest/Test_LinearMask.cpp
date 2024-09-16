#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <optional>

#include "plato/filter/extension/LinearMask.hpp"
#include "plato/filter/extension/test_utilities/LinearMaskTestUtility.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"

namespace plato::filter::extension::parallel_unittest
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
    const LinearMask tLinearMask = test_utilities::create_simple_linear_mask();

    /* 2/3          1/3             0           0
       1/4         1/2            1/4        0
       0            1/4            1/2        1/4
       0            0               1/3         2/3*/

    const auto [tRows, tCols] = tLinearMask.size();
    ASSERT_EQ(tRows, 4);
    ASSERT_EQ(tCols, 4);
    {
        const std::vector<double> tProjection{0, 0, 1, 0};
        const auto tResult = tLinearMask.matrixMultiply(tProjection);
        ASSERT_EQ(tResult.size(), 4u);
        EXPECT_DOUBLE_EQ(tResult[0], 0);
        EXPECT_DOUBLE_EQ(tResult[1], 0.25);
        EXPECT_DOUBLE_EQ(tResult[2], 0.5);
        EXPECT_DOUBLE_EQ(tResult[3], 1.0 / 3.0);
    }
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

}  // namespace plato::filter::extension::parallel_unittest
