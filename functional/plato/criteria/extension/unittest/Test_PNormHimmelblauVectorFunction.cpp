#include <gtest/gtest.h>

#include "plato/core/Compose.hpp"
#include "plato/criteria/extension/PNorm.hpp"
#include "plato/criteria/extension/unittest/HimmelblauVectorFunction.hpp"
#include "plato/test_utilities/Himmelblau.hpp"

namespace plato::criteria::extension::unittest
{

namespace
{
const auto kHimmelblauOne = test_utilities::Himmelblau{};
const auto kHimmelblauTwo = test_utilities::Himmelblau{2, 20};
const auto kVectorHimmelblau = make_vector_himmelblau_dynamic_vector_objective_function(
    FirstHimmelblau{kHimmelblauOne}, SecondHimmelblau{kHimmelblauTwo});

const auto kPoint = linear_algebra::DynamicVector<double>{1.4, 1.6};
const auto kVectorValue = kVectorHimmelblau.evaluate<core::evaluation::kFunction>(kPoint).stdVector();

const auto kDirection = linear_algebra::DynamicVector<double>{1, 2};
const auto kDirectionTimesJacobianMultiplier = kVectorHimmelblau.evaluate<core::evaluation::kFirstDerivative>(kPoint)
                                                   .mVectorTimesJacobianFunction(kDirection)
                                                   .stdVector();
const auto kDirectionTimesAdjointJacobianMultiplier =
    kVectorHimmelblau.evaluate<core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>(kPoint)
        .mValue.mVectorTimesJacobianFunction(kDirection)
        .stdVector();

const auto kDerivativeHimmelblauOne = kHimmelblauOne.df(kPoint.stdVector().front(), kPoint.stdVector().back());
const auto kDerivativeHimmelblauTwo = kHimmelblauTwo.df(kPoint.stdVector().front(), kPoint.stdVector().back());

}  // namespace

TEST(PNormHimmelblau, HimmelblauVectorFunction)
{
    ASSERT_EQ(kVectorValue.size(), 2U);
    EXPECT_EQ(kVectorValue.front(), kHimmelblauOne.f(kPoint.stdVector().front(), kPoint.stdVector().back()));
    EXPECT_EQ(kVectorValue.back(), kHimmelblauTwo.f(kPoint.stdVector().front(), kPoint.stdVector().back()));

    const auto tGoldDirectionTimesJacobianFirstEntry =
        kDirection.stdVector().front() * kDerivativeHimmelblauOne.mData.at(0) +
        kDirection.stdVector().back() * kDerivativeHimmelblauTwo.mData.at(0);
    const auto tGoldDirectionTimesJacobianSecondEntry =
        kDirection.stdVector().front() * kDerivativeHimmelblauOne.mData.at(1) +
        kDirection.stdVector().back() * kDerivativeHimmelblauTwo.mData.at(1);

    ASSERT_EQ(kDirectionTimesJacobianMultiplier.size(), 2U);
    EXPECT_EQ(kDirectionTimesJacobianMultiplier.front(), tGoldDirectionTimesJacobianFirstEntry);
    EXPECT_EQ(kDirectionTimesJacobianMultiplier.back(), tGoldDirectionTimesJacobianSecondEntry);

    const auto tGoldDirectionTimesAdjointJacobianFirstEntry =
        kDirection.stdVector().front() * kDerivativeHimmelblauOne.mData.at(0) +
        kDirection.stdVector().back() * kDerivativeHimmelblauOne.mData.at(1);
    const auto tGoldDirectionTimesAdjointJacobianSecondEntry =
        kDirection.stdVector().front() * kDerivativeHimmelblauTwo.mData.at(0) +
        kDirection.stdVector().back() * kDerivativeHimmelblauTwo.mData.at(1);

    ASSERT_EQ(kDirectionTimesAdjointJacobianMultiplier.size(), 2U);
    EXPECT_EQ(kDirectionTimesAdjointJacobianMultiplier.front(), tGoldDirectionTimesAdjointJacobianFirstEntry);
    EXPECT_EQ(kDirectionTimesAdjointJacobianMultiplier.back(), tGoldDirectionTimesAdjointJacobianSecondEntry);
}

TEST(PNormHimmelblau, HimmelblauPNormComposition)
{
    constexpr double tPower = 1.0;
    const auto tPnorm = criteria::extension::make_p_norm_function(tPower);
    const auto tComposition = core::compose(tPnorm, kVectorHimmelblau);
    const auto tValue = tComposition.evaluate<core::evaluation::kFunction>(kPoint);
    EXPECT_DOUBLE_EQ(tValue, kVectorValue.at(0) + kVectorValue.at(1));

    const auto tGradient = tComposition.evaluate<core::evaluation::kFirstDerivative>(kPoint).stdVector();

    // p=1, abs(x)*sign(x) = x
    const auto tGoldGradient =
        std::vector<double>{kDerivativeHimmelblauOne.mData[0] + kDerivativeHimmelblauTwo.mData[0],
                            kDerivativeHimmelblauOne.mData[1] + kDerivativeHimmelblauTwo.mData[1]};

    EXPECT_EQ(tGoldGradient, tGradient);
}

}  // namespace plato::criteria::extension::unittest
