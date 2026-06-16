#include <gtest/gtest.h>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/QuadratureRules.hpp"

namespace plato::third_party_integration::common::unittest
{
namespace
{
template <typename Function>
void test_integration_of_1_d_polynomial(const Function& aPolynomialFunction,
                                        const double aGold,
                                        const test_utilities::TestContext& aTestContext)
{
    auto tValue = 0.0;
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
    constexpr auto tQuadraturePoints = common::kDegree3OneDimensionalQuadraturePoints;
    const auto tResult =
        tensor_product_unit_integral<kDimensions, tQuadraturePoints.size(), tQuadraturePoints>(aMonomialFunction);
    EXPECT_DOUBLE_EQ(tResult, aGold) << aTestContext;
}

}  // namespace

TEST(QuadratureRule, 1DIntegration)
{
    {
        constexpr double tGold{2.0};
        const auto tPolynomial = [](const double /*aXCoordinate*/) { return 1.0; };
        test_integration_of_1_d_polynomial(tPolynomial, tGold, TEST_CONTEXT("1"));
    }

    {
        constexpr double tGold{8.0 / 3.0};
        const auto tPolynomial = [](const double aXCoordinate) { return aXCoordinate * aXCoordinate + 1.0; };
        test_integration_of_1_d_polynomial(tPolynomial, tGold, TEST_CONTEXT("x^2 + 1"));
    }

    {
        constexpr double tGold{2.0};
        const auto tPolynomial = [](const double aXCoordinate)
        { return aXCoordinate * aXCoordinate * aXCoordinate + 1.0; };
        test_integration_of_1_d_polynomial(tPolynomial, tGold, TEST_CONTEXT("x^3 + 1"));
    }

    {
        constexpr double tGold{8.0 / 3.0};
        const auto tPolynomial = [](const double aXCoordinate)
        { return aXCoordinate * aXCoordinate * aXCoordinate + aXCoordinate * aXCoordinate + aXCoordinate + 1.0; };
        test_integration_of_1_d_polynomial(tPolynomial, tGold, TEST_CONTEXT("x^3 + x^2 + x + 1"));
    }
}

TEST(QuadratureRule, 2DIntegration)
{
    {
        constexpr double tGold{4.0};
        const auto tMonomial = [](const double /*aXCoordinate*/, const double /*aYCoordinate*/) { return 1.0; };
        test_integration_of_multidimensional_monomial<2>(tMonomial, tGold, TEST_CONTEXT("1"));
    }

    {
        constexpr double tGold{4.0 / 3.0};
        const auto tMonomial = [](const double aXCoordinate, const double /*aYCoordinate*/)
        { return aXCoordinate * aXCoordinate; };
        test_integration_of_multidimensional_monomial<2>(tMonomial, tGold, TEST_CONTEXT("x^2"));
    }

    {
        constexpr double tGold{4.0 / 3.0};
        const auto tMonomial = [](const double /*aXCoordinate*/, const double aYCoordinate)
        { return aYCoordinate * aYCoordinate; };
        test_integration_of_multidimensional_monomial<2>(tMonomial, tGold, TEST_CONTEXT("y^2"));
    }

    {
        constexpr double tGold{4.0 / 9.0};
        const auto tMonomial = [](const double aXCoordinate, const double aYCoordinate)
        { return aXCoordinate * aXCoordinate * aYCoordinate * aYCoordinate; };
        test_integration_of_multidimensional_monomial<2>(tMonomial, tGold, TEST_CONTEXT("x^2 * y^2"));
    }
}

TEST(QuadratureRule, 3DIntegration)
{
    {
        constexpr double tGold{8.0};
        const auto tMonomial = [](const double /*aXCoordinate*/, const double /*aYCoordinate*/,
                                  const double /*aZCoordinate*/) { return 1.0; };
        test_integration_of_multidimensional_monomial<3>(tMonomial, tGold, TEST_CONTEXT("1"));
    }

    {
        constexpr double tGold{8.0 / 3.0};
        const auto tMonomial = [](const double aXCoordinate, const double /*aYCoordinate*/,
                                  const double /*aZCoordinate*/) { return aXCoordinate * aXCoordinate; };
        test_integration_of_multidimensional_monomial<3>(tMonomial, tGold, TEST_CONTEXT("x^2"));
    }

    {
        constexpr double tGold{8.0 / 3.0};
        const auto tMonomial = [](const double /*aXCoordinate*/, const double aYCoordinate,
                                  const double /*aZCoordinate*/) { return aYCoordinate * aYCoordinate; };
        test_integration_of_multidimensional_monomial<3>(tMonomial, tGold, TEST_CONTEXT("y^2"));
    }

    {
        constexpr double tGold{8.0 / 3.0};
        const auto tMonomial = [](const double /*aXCoordinate*/, const double /*aYCoordinate*/,
                                  const double aZCoordinate) { return aZCoordinate * aZCoordinate; };
        test_integration_of_multidimensional_monomial<3>(tMonomial, tGold, TEST_CONTEXT("z^2"));
    }

    {
        constexpr double tGold{8.0 / 27.0};
        const auto tMonomial = [](const double aXCoordinate, const double aYCoordinate, const double aZCoordinate)
        { return aXCoordinate * aXCoordinate * aYCoordinate * aYCoordinate * aZCoordinate * aZCoordinate; };
        test_integration_of_multidimensional_monomial<3>(tMonomial, tGold, TEST_CONTEXT("x^2 * y^2 * z^2"));
    }
}
}  // namespace plato::third_party_integration::common::unittest
