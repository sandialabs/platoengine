#include <gtest/gtest.h>

#include <iterator>

#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::linear_algebra::unittest
{
TEST(JacobianMultiplier, Identity)
{
    const JacobianMultiplier tIdentityJacobian = {/*.mVectorTimesJacobianFunction=*/
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
    const auto tVector = DynamicVector{1.0, 2.0, 3.0};

    const auto tResult = tJacobianMultiplierProduct.mVectorTimesJacobianFunction(tVector);

    const auto tDoubled = tJacobianMultiplierDouble.mVectorTimesJacobianFunction(tVector);
    const auto tExpected = tJacobianMultiplierSquare.mVectorTimesJacobianFunction(tDoubled);

    EXPECT_EQ(tResult.stdVector(), tExpected.stdVector());
}

TEST(AdjointJacobianMultiplier, MultiplicationWithVector)
{
    const auto tJacobianMultiplierSquare =
        JacobianMultiplier{[](const DynamicVector<double>& aV) { return square(aV); }};
    const auto tAdjointJacobianMultiplierSquare = AdjointJacobianMultiplier{tJacobianMultiplierSquare};

    // Result should be the same as with JacobianMultiplier, AdjointJacobianMultiplier is just a strong type
    const auto tVector = DynamicVector{1.0, 2.0};
    const auto tExpected = tVector * tJacobianMultiplierSquare;
    const auto tResult = tVector * tAdjointJacobianMultiplierSquare;
    EXPECT_EQ(tExpected.stdVector(), tResult.stdVector());
}

TEST(AdjointJacobianMultiplier, MultiplicationWithAdjointJacobian)
{
    const auto tAdjointJacobianMultiplierSquare =
        AdjointJacobianMultiplier{JacobianMultiplier{[](const DynamicVector<double>& aV) { return square(aV); }}};
    const auto tAdjointJacobianMultiplierDouble =
        AdjointJacobianMultiplier{JacobianMultiplier{[](const DynamicVector<double>& aV) { return double_value(aV); }}};

    const auto tAdjointProduct = tAdjointJacobianMultiplierSquare * tAdjointJacobianMultiplierDouble;
    const auto tVector = DynamicVector{1.0, 2.0};
    const auto tExpected = (tVector * tAdjointJacobianMultiplierSquare) * tAdjointJacobianMultiplierDouble;
    const auto tResult = tVector * tAdjointProduct;
    EXPECT_EQ(tExpected.stdVector(), tResult.stdVector());
}

TEST(AdjointJacobianMultiplier, MakeAdjointJacobianMultiplier)
{
    const auto tAdjointJacobianViaCtor =
        AdjointJacobianMultiplier{JacobianMultiplier{[](const DynamicVector<double>& aV) { return square(aV); }}};
    const auto tAdjointJacobianViaMake =
        make_adjoint_jacobian_multiplier([](const DynamicVector<double>& aV) { return square(aV); });

    const auto tVector = DynamicVector{-10.0, 22.0};
    const auto tResultViaCtor = tVector * tAdjointJacobianViaCtor;
    const auto tResultViaMake = tVector * tAdjointJacobianViaMake;
    EXPECT_EQ(tResultViaCtor, tResultViaMake);
}
}  // namespace plato::linear_algebra::unittest
