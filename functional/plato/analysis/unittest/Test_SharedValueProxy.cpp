#include <gtest/gtest.h>

#include <iterator>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshOperators.hpp"
#include "plato/analysis/SharedValueProxy.hpp"

namespace plato::analysis::unittest
{
namespace
{
const auto kDensity0 = ScalarFieldValue{0, 0, 0.0};
const auto kDensity1 = ScalarFieldValue{1, 1, 1.0};
const auto kDensity2 = ScalarFieldValue{2, 2, 2.0};

using VectorType = std::vector<ScalarFieldValue>;
using SharedDensityProxy = SharedValueProxy<ScalarFieldValue, typename VectorType::iterator>;

struct AssignDensity
{
    void operator()(ScalarFieldValue& aScalarField, const double aDensity) const { aScalarField.mValue = aDensity; }
};
}  // namespace

TEST(SharedDensityProxy, AssignmentOperator1Iterator)
{
    auto tVector = VectorType{kDensity0, kDensity1};
    auto tProxy = SharedDensityProxy{{tVector.begin()}};

    ASSERT_EQ(tProxy.mIterators.size(), 1U);
    EXPECT_EQ(*tProxy.mIterators.front(), kDensity0);

    tProxy = kDensity1;
    EXPECT_EQ(*tProxy.mIterators.front(), kDensity1);
}

TEST(SharedDensityProxy, AssignmentOperator2Iterators)
{
    auto tVector1 = VectorType{kDensity0};
    auto tVector2 = VectorType{kDensity1};
    auto tProxy = SharedDensityProxy{{tVector1.begin(), tVector2.begin()}};

    ASSERT_EQ(tProxy.mIterators.size(), 2U);
    EXPECT_EQ(*tProxy.mIterators.front(), kDensity0);
    EXPECT_EQ(*tProxy.mIterators.back(), kDensity1);

    tProxy = kDensity2;
    EXPECT_EQ(*tProxy.mIterators.front(), kDensity2);
    EXPECT_EQ(*tProxy.mIterators.back(), kDensity2);
}

TEST(SharedDensityProxy, CastToDensity)
{
    auto tVector1 = VectorType{kDensity1};
    auto tVector2 = VectorType{kDensity1};
    auto tProxy = SharedDensityProxy{{tVector1.begin(), tVector2.begin()}};

    EXPECT_EQ(static_cast<ScalarFieldValue>(tProxy), kDensity1);
}

TEST(SharedDensityProxy, AssignmentPolicy)
{
    using SharedDensityProxyValueAssignment =
        SharedValueProxy<ScalarFieldValue, typename VectorType::iterator, AssignDensity>;
    auto tVector1 = VectorType{kDensity0, kDensity1};
    auto tVector2 = VectorType{kDensity1};
    auto tProxy = SharedDensityProxyValueAssignment{{std::next(tVector1.begin()), tVector2.begin()}};

    constexpr auto tNewValue = 42.0;
    tProxy = tNewValue;
    EXPECT_EQ(tVector1.back().mValue, tNewValue);
    EXPECT_EQ(tVector1.back().mGlobalMeshEntityID, kDensity1.mGlobalMeshEntityID);
    EXPECT_EQ(tVector1.back().mDesignVariableVectorIndex, kDensity1.mDesignVariableVectorIndex);
    EXPECT_EQ(tVector2.front().mValue, tNewValue);
    EXPECT_EQ(tVector2.front().mGlobalMeshEntityID, kDensity1.mGlobalMeshEntityID);
    EXPECT_EQ(tVector2.front().mDesignVariableVectorIndex, kDensity1.mDesignVariableVectorIndex);
}

TEST(SharedDensityProxy, Empty)
{
    auto tProxy = SharedDensityProxy{};
    EXPECT_TRUE(tProxy.empty());

    auto tVector1 = VectorType{kDensity0};
    auto tVector2 = VectorType{kDensity1};
    tProxy = SharedDensityProxy{{tVector1.begin(), tVector2.begin()}};
    EXPECT_FALSE(tProxy.empty());
}

}  // namespace plato::analysis::unittest
