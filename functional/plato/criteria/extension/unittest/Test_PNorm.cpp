#include <gtest/gtest.h>

#include <cmath>

#include "plato/criteria/extension/PNorm.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/Containers.hpp"
#include "plato/test_utilities/GradientChecker.hpp"

namespace plato::criteria::extension::unittest
{

namespace
{

const auto kTestVector = std::vector<double>{0, 10, -1.1, -2.2, -3.3, -4.4, 5, 4.4, 3.3, 2.2, 1.1};

constexpr double kTolerance = 5e-16;
constexpr double kGoldPNormOne = 37;
constexpr double kGoldPNormExpressionTwo = 197.6;
constexpr double kGoldPNormExpressionThree = 1391.2;

const auto kGoldDerivativeOne = std::vector<double>{0, 1, -1, -1, -1, -1, 1, 1, 1, 1, 1};
const auto kGoldDerivativeTwo = std::vector<double>{0,
                                                    0.71138799153059,
                                                    -0.0782526790683649,
                                                    -0.15650535813673,
                                                    -0.234758037205095,
                                                    -0.31301071627346,
                                                    0.355693995765295,
                                                    0.31301071627346,
                                                    0.234758037205095,
                                                    0.15650535813673,
                                                    0.0782526790683649};

const auto kGoldDerivativeThree = std::vector<double>{0,
                                                      0.802429624857272,
                                                      -0.00970939846077299,
                                                      -0.038837593843092,
                                                      -0.0873845861469569,
                                                      -0.155350375372368,
                                                      0.200607406214318,
                                                      0.155350375372368,
                                                      0.0873845861469569,
                                                      0.038837593843092,
                                                      0.00970939846077299};

}  // namespace

TEST(PNormDetail, PNormExpression)
{
    {
        constexpr double tPower = 1;
        const auto tResult = detail::p_norm_expression(kTestVector, tPower);
        EXPECT_DOUBLE_EQ(kGoldPNormOne, tResult);
    }
    {
        constexpr double tPower = 2;
        const auto tResult = detail::p_norm_expression(kTestVector, tPower);
        EXPECT_DOUBLE_EQ(kGoldPNormExpressionTwo, tResult);
    }
    {
        constexpr double tPower = 3;
        const auto tResult = detail::p_norm_expression(kTestVector, tPower);
        EXPECT_DOUBLE_EQ(kGoldPNormExpressionThree, tResult);
    }
}

TEST(PNormDetail, Value)
{
    {
        constexpr double tPower = 1;
        const auto tResult = detail::p_norm_value(kTestVector, tPower);
        EXPECT_DOUBLE_EQ(kGoldPNormOne, tResult);
    }
    {
        constexpr double tPower = 2;
        const auto tResult = detail::p_norm_value(kTestVector, tPower);
        EXPECT_NEAR(std::sqrt(kGoldPNormExpressionTwo), tResult, kTolerance);
    }
    {
        constexpr double tPower = 3;
        const auto tResult = detail::p_norm_value(kTestVector, tPower);
        EXPECT_NEAR(std::pow(kGoldPNormExpressionThree, 1.0 / tPower), tResult, kTolerance);
    }
}

TEST(PNormDetail, Gradient)
{
    {
        constexpr double tPower = 1;
        const auto tResult = detail::p_norm_gradient(kTestVector, tPower);
        EXPECT_EQ(kGoldDerivativeOne, tResult);
    }
    {
        constexpr double tPower = 2;
        const auto tResult = detail::p_norm_gradient(kTestVector, tPower);
        test_utilities::expect_container_entries_near(kGoldDerivativeTwo, tResult, kTolerance,
                                                      TEST_CONTEXT("P Norm 2 Gradient"));
    }
    {
        constexpr double tPower = 3;
        const auto tResult = detail::p_norm_gradient(kTestVector, tPower);
        test_utilities::expect_container_entries_near(kGoldDerivativeThree, tResult, kTolerance,
                                                      TEST_CONTEXT("P Norm 3 Gradient"));
    }
}

TEST(PNorm, MakePNormFunction)
{
    const auto tVector = linear_algebra::DynamicVector<double>{kTestVector};
    constexpr double tPower = 1;
    const auto tFunction = make_p_norm_function(tPower);

    const auto tValueResult = tFunction.evaluate<core::evaluation::kFunction>(tVector);
    EXPECT_EQ(tValueResult, kGoldPNormOne);

    const auto tGradientResult = tFunction.evaluate<core::evaluation::kFirstDerivative>(tVector).stdVector();
    EXPECT_EQ(tGradientResult, kGoldDerivativeOne);
}

TEST(PNorm, GradientCheck)
{
    constexpr auto tAbsoluteError = 5e-2;
    constexpr double tPower = 4;
    const auto tF = [](const linear_algebra::DynamicVector<double>& aX)
    { return detail::p_norm_value(aX.stdVector(), tPower); };
    const auto tDf =
        [](const linear_algebra::DynamicVector<double>& aX, const linear_algebra::DynamicVector<double>& aV)
    {
        const auto tGradient = linear_algebra::DynamicVector<double>{detail::p_norm_gradient(aX.stdVector(), tPower)};
        return tGradient.dot(aV);
    };

    const auto tGradientCheckParameters = test_utilities::GradientCheckParameters{0.5, 10, .050};
    const auto tGradientCheck = test_utilities::GradientChecker{tF, tDf};
    const auto tX = linear_algebra::DynamicVector<double>{1.0, 2.0, -4.0, 0.0, -10.0};
    const auto tDirection = linear_algebra::DynamicVector<double>{.10, -.10, 0.05, 0.03, -0.09};

    EXPECT_NEAR(tGradientCheck.maxFirstOrderTruncationError(tX, tDirection, tGradientCheckParameters), 0.0,
                tAbsoluteError)
        << tGradientCheck.table(tX, tDirection, tGradientCheckParameters);
}

}  // namespace plato::criteria::extension::unittest
