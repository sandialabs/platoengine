#include <gtest/gtest.h>

#include "plato/mesh/MeshDesignVariables.hpp"
#include "plato/mesh/SharedValueProxy.hpp"
#include "plato/mesh/unittest/Utilities.hpp"

namespace plato::mesh::unittest
{
namespace
{
const auto tDensity0 = Density{0, 0, 0.0};
const auto tDensity1 = Density{1, 1, 1.0};
const auto tDensity2 = Density{2, 2, 2.0};

using VectorType = std::vector<Density>;
using SharedDensityProxy = SharedValueProxy<Density, typename VectorType::iterator>;
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

    EXPECT_EQ(static_cast<Density>(tProxy), tDensity1);
}
}  // namespace plato::mesh::unittest
