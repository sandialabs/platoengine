#include <gtest/gtest.h>

#include "plato/filter/extension/kernel_filters/ReliableMeshZeroValue.hpp"

namespace plato::filter::extension::kernel_filters::unittest
{
TEST(ReliableMeshZeroValue, RegressionValue)
{
    constexpr auto tRegressionValue = 1e-6;
    const auto tResult = reliable_mesh_zero_value();
    EXPECT_EQ(tResult, tRegressionValue);
}

}  // namespace plato::filter::extension::kernel_filters::unittest
