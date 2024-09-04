#include <gtest/gtest.h>

#include "plato/design_variables/MeshDesignVariables.hpp"
#include "plato/design_variables/SharedValueProxy.hpp"
#include "plato/design_variables/unittest/Utilities.hpp"

namespace plato::design_variables::unittest
{
namespace
{
const auto tDensity0 = ScalarFieldValue{0, 0, 0.0};
const auto tDensity1 = ScalarFieldValue{1, 1, 1.0};
const auto tDensity2 = ScalarFieldValue{2, 2, 2.0};

using VectorType = std::vector<ScalarFieldValue>;
using SharedDensityProxy = SharedValueProxy<ScalarFieldValue, typename VectorType::iterator>;
}  // namespace

TEST(SharedDensityProxy, AssignmentOperator1Iterator)
{
    auto tVector = VectorType{tDensity0, tDensity1};
    auto tProxy = SharedDensityProxy{{tVector.begin()}};

    ASSERT_EQ(tProxy.mIterators.size(), 1U);
    EXPECT_EQ(*tProxy.mIterators.front(), tDensity0);

    tProxy = tDensity1;
    EXPECT_EQ(*tProxy.mIterators.front(), tDensity1);
}

TEST(SharedDensityProxy, AssignmentOperator2Iterators)
{
    auto tVector1 = VectorType{tDensity0};
    auto tVector2 = VectorType{tDensity1};
    auto tProxy = SharedDensityProxy{{tVector1.begin(), tVector2.begin()}};

    ASSERT_EQ(tProxy.mIterators.size(), 2U);
    EXPECT_EQ(*tProxy.mIterators.front(), tDensity0);
    EXPECT_EQ(*tProxy.mIterators.back(), tDensity1);

    tProxy = tDensity2;
    EXPECT_EQ(*tProxy.mIterators.front(), tDensity2);
    EXPECT_EQ(*tProxy.mIterators.back(), tDensity2);
}

TEST(SharedDensityProxy, CastToDensity)
{
    auto tVector1 = VectorType{tDensity1};
    auto tVector2 = VectorType{tDensity1};
    auto tProxy = SharedDensityProxy{{tVector1.begin(), tVector2.begin()}};

    EXPECT_EQ(static_cast<ScalarFieldValue>(tProxy), tDensity1);
}
}  // namespace plato::design_variables::unittest
