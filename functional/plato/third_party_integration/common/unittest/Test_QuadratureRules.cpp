#include <gtest/gtest.h>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/QuadratureRules.hpp"

namespace plato::third_party_integration::common::unittest
{
namespace
{
template <typename Function>
void test_integration_of_1D_polynomial(const Function& aPolynomialFunction,
                                       const double aGold,
                                       const test_utilities::TestContext& aTestContext)
{
    auto tValue = double{0.0};
    for (const auto& tPoint : kDegree3OneDimensionalQuadraturePoints)
    {
        tValue += tPoint.mWeight * aPolynomialFunction(tPoint.mPoint);
    }

    EXPECT_EQ(tValue, aGold) << aTestContext;
}

template <int kDimensions, typename Function>
void test_integration_of_multidimensional_monomial(const Function& aMonomialFunction,
                                                   const double aGold,
                                                   const test_utilities::TestContext& aTestContext)
{
    constexpr auto kQuadraturePoints = common::kDegree3OneDimensionalQuadraturePoints;
    const auto tResult =
        tensor_product_unit_integral<kDimensions, kQuadraturePoints.size(), kQuadraturePoints>(aMonomialFunction);
    EXPECT_DOUBLE_EQ(tResult, aGold) << aTestContext;
}

}  // namespace

TEST(QuadratureRule, 1DIntegration)
{
    {
        constexpr double tGold{2.0};
        const auto tPolynomial = [](const double /*aX*/) { return 1.0; };
        test_integration_of_1D_polynomial(tPolynomial, tGold, TEST_CONTEXT("1"));
    }

    {
        constexpr double tGold{8.0 / 3.0};
        const auto tPolynomial = [](const double aX) { return aX * aX + 1.0; };
        test_integration_of_1D_polynomial(tPolynomial, tGold, TEST_CONTEXT("x^2 + 1"));
    }

    {
        constexpr double tGold{2.0};
        const auto tPolynomial = [](const double aX) { return aX * aX * aX + 1.0; };
        test_integration_of_1D_polynomial(tPolynomial, tGold, TEST_CONTEXT("x^3 + 1"));
    }

    {
        constexpr double tGold{8.0 / 3.0};
        const auto tPolynomial = [](const double aX) { return aX * aX * aX + aX * aX + aX + 1.0; };
        test_integration_of_1D_polynomial(tPolynomial, tGold, TEST_CONTEXT("x^3 + x^2 + x + 1"));
    }
}

TEST(QuadratureRule, 2DIntegration)
{
    {
        constexpr double tGold{4.0};
        const auto tMonomial = [](const double /*aX*/, const double /*aY*/) { return 1.0; };
        test_integration_of_multidimensional_monomial<2>(tMonomial, tGold, TEST_CONTEXT("1"));
    }

    {
        constexpr double tGold{4.0 / 3.0};
        const auto tMonomial = [](const double aX, const double /*aY*/) { return aX * aX; };
        test_integration_of_multidimensional_monomial<2>(tMonomial, tGold, TEST_CONTEXT("x^2"));
    }

    {
        constexpr double tGold{4.0 / 3.0};
        const auto tMonomial = [](const double /*aX*/, const double aY) { return aY * aY; };
        test_integration_of_multidimensional_monomial<2>(tMonomial, tGold, TEST_CONTEXT("y^2"));
    }

    {
        constexpr double tGold{4.0 / 9.0};
        const auto tMonomial = [](const double aX, const double aY) { return aX * aX * aY * aY; };
        test_integration_of_multidimensional_monomial<2>(tMonomial, tGold, TEST_CONTEXT("x^2 * y^2"));
    }
}

TEST(QuadratureRule, 3DIntegration)
{
    {
        constexpr double tGold{8.0};
        const auto tMonomial = [](const double /*aX*/, const double /*aY*/, const double /*aZ*/) { return 1.0; };
        test_integration_of_multidimensional_monomial<3>(tMonomial, tGold, TEST_CONTEXT("1"));
    }

    {
        constexpr double tGold{8.0 / 3.0};
        const auto tMonomial = [](const double aX, const double /*aY*/, const double /*aZ*/) { return aX * aX; };
        test_integration_of_multidimensional_monomial<3>(tMonomial, tGold, TEST_CONTEXT("x^2"));
    }

    {
        constexpr double tGold{8.0 / 3.0};
        const auto tMonomial = [](const double /*aX*/, const double aY, const double /*aZ*/) { return aY * aY; };
        test_integration_of_multidimensional_monomial<3>(tMonomial, tGold, TEST_CONTEXT("y^2"));
    }

    {
        constexpr double tGold{8.0 / 3.0};
        const auto tMonomial = [](const double /*aX*/, const double /*aY*/, const double aZ) { return aZ * aZ; };
        test_integration_of_multidimensional_monomial<3>(tMonomial, tGold, TEST_CONTEXT("z^2"));
    }

    {
        constexpr double tGold{8.0 / 27.0};
        const auto tMonomial = [](const double aX, const double aY, const double aZ)
        { return aX * aX * aY * aY * aZ * aZ; };
        test_integration_of_multidimensional_monomial<3>(tMonomial, tGold, TEST_CONTEXT("x^2 * y^2 * z^2"));
    }
}
}  // namespace plato::third_party_integration::common::unittest
