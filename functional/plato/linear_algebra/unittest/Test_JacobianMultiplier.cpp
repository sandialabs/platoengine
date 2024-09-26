#include <gtest/gtest.h>

#include <iterator>

#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::linear_algebra::unittest
{
TEST(JacobianMultiplier, Identity)
{
    const JacobianMultiplier tIdentityJacobian = {/*.mJacobianTimesVectorFunction=*/
                                                  [](const DynamicVector<double>& aV) { return aV; }};

    const auto tVec = DynamicVector<double>{1.0, 2.0};
    const DynamicVector<double> tRes = tVec * tIdentityJacobian;
    EXPECT_EQ(tRes.stdVector(), tVec.stdVector());
}

namespace
{
DynamicVector<double> square(const DynamicVector<double>& aDynamicVector)
{
    std::vector<double> tSquare;
    tSquare.reserve(aDynamicVector.size());
    std::transform(aDynamicVector.stdVector().begin(), aDynamicVector.stdVector().end(), std::back_inserter(tSquare),
                   [](const auto& x) { return x * x; });

    return DynamicVector<double>{std::move(tSquare)};
}

DynamicVector<double> double_value(const DynamicVector<double>& aDynamicVector)
{
    std::vector<double> tDouble;
    tDouble.reserve(aDynamicVector.size());
    std::transform(aDynamicVector.stdVector().begin(), aDynamicVector.stdVector().end(), std::back_inserter(tDouble),
                   [](const auto& x) { return 2.0 * x; });

    return DynamicVector<double>{std::move(tDouble)};
}

}  // namespace

TEST(JacobianMultiplier, Multiplication)
{
    const auto tJacobianMultiplierSquare =
        JacobianMultiplier{[](const DynamicVector<double>& aV) { return square(aV); }};
    const auto tJacobianMultiplierDouble =
        JacobianMultiplier{[](const DynamicVector<double>& aV) { return double_value(aV); }};

    const auto tJacobianMultiplierProduct = tJacobianMultiplierDouble * tJacobianMultiplierSquare;
    const DynamicVector<double> tEntries({1, 2, 3});

    const auto tResult = tJacobianMultiplierProduct.mJacobianTimesVectorFunction(tEntries);

    const auto tDoubled = tJacobianMultiplierDouble.mJacobianTimesVectorFunction(tEntries);
    const auto tExpected = tJacobianMultiplierSquare.mJacobianTimesVectorFunction(tDoubled);

    EXPECT_EQ(tResult.stdVector(), tExpected.stdVector());
}

}  // namespace plato::linear_algebra::unittest
