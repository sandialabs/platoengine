#include <gtest/gtest.h>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshOperators.hpp"
#include "plato/analysis/SharedValueProxy.hpp"

namespace plato::analysis::unittest
{
namespace
{
const auto tDensity0 = ScalarFieldValue{0, 0, 0.0};
const auto tDensity1 = ScalarFieldValue{1, 1, 1.0};
const auto tDensity2 = ScalarFieldValue{2, 2, 2.0};

using VectorType = std::vector<ScalarFieldValue>;
using SharedDensityProxy = SharedValueProxy<ScalarFieldValue, typename VectorType::iterator>;

struct AssignDensity
{
    void operator()(ScalarFieldValue& aScalarField, const double aDensity) const { aScalarField.mValue = aDensity; }
};
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

TEST(SharedDensityProxy, AssignmentPolicy)
{
    using SharedDensityProxyValueAssignment =
        SharedValueProxy<ScalarFieldValue, typename VectorType::iterator, AssignDensity>;
    auto tVector1 = VectorType{tDensity0, tDensity1};
    auto tVector2 = VectorType{tDensity1};
    auto tProxy = SharedDensityProxyValueAssignment{{std::next(tVector1.begin()), tVector2.begin()}};

    constexpr auto tNewValue = double{42.0};
    tProxy = tNewValue;
    EXPECT_EQ(tVector1.back().mValue, tNewValue);
    EXPECT_EQ(tVector1.back().mGlobalMeshEntityID, tDensity1.mGlobalMeshEntityID);
    EXPECT_EQ(tVector1.back().mDesignVariableVectorIndex, tDensity1.mDesignVariableVectorIndex);
    EXPECT_EQ(tVector2.front().mValue, tNewValue);
    EXPECT_EQ(tVector2.front().mGlobalMeshEntityID, tDensity1.mGlobalMeshEntityID);
    EXPECT_EQ(tVector2.front().mDesignVariableVectorIndex, tDensity1.mDesignVariableVectorIndex);
}

}  // namespace plato::analysis::unittest
