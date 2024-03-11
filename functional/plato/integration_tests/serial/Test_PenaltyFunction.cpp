#include <gtest/gtest.h>

#include "plato/core/Compose.hpp"
#include "plato/integration_tests/utilities/DynamicVectorPenaltyFunction.hpp"
#include "plato/integration_tests/utilities/DynamicVectorRosenbrockFunction.hpp"
#include "plato/test_utilities/Penalty.hpp"

namespace plato::integration_tests::serial
{
TEST(PenaltyFunction, ValueAndJacobian)
{
    namespace pft = plato::test_utilities;
    namespace pfitu = plato::integration_tests::utilities;

    constexpr double tXMin = 0.5e-2;
    constexpr double tPower = 2.0;
    const auto tPenalty = pfitu::make_penalty_dynamic_vector_function(pft::Penalty{tXMin, tPower});

    // Function and derivative at 1 and 0
    const auto tControl = linear_algebra::DynamicVector{1.0, 0.0};
    const linear_algebra::DynamicVector tFOfControl = tPenalty.f(tControl);
    EXPECT_EQ(tFOfControl[0], 1.0);
    EXPECT_EQ(tFOfControl[1], tXMin);

    const auto tdFOfControl = tPenalty.df(tControl);
    EXPECT_EQ(tdFOfControl.mJacobian(0, 0), tPower * (1.0 - tXMin));
    EXPECT_EQ(tdFOfControl.mJacobian(0, 1), 0.0);
    EXPECT_EQ(tdFOfControl.mJacobian(1, 0), 0.0);
    EXPECT_EQ(tdFOfControl.mJacobian(1, 1), 0.0);

    const linear_algebra::DynamicVector tColumn0 = tdFOfControl.column(0);
    EXPECT_EQ(tColumn0[0], tPower * (1.0 - tXMin));
    EXPECT_EQ(tColumn0[1], 0.0);

    const linear_algebra::DynamicVector tColumn1 = tdFOfControl.column(1);
    EXPECT_EQ(tColumn1[0], 0.0);
    EXPECT_EQ(tColumn1[1], 0.0);
}

TEST(PenaltyFunction, Multiplication)
{
    namespace pft = plato::test_utilities;

    const auto tX = linear_algebra::DynamicVector{1.0, 2.0};
    const auto tA = utilities::DynamicVectorJacobian{pft::makeTwoDMatrix(1.0, 2.0, 3.0, 4.0)};
    const auto tb = tX * tA;
    EXPECT_EQ(tb[0], 7.0);
    EXPECT_EQ(tb[1], 10.0);
}

TEST(PenaltyFunction, Composition)
{
    namespace pft = plato::test_utilities;
    namespace pfitu = plato::integration_tests::utilities;

    const auto tPenalty = pfitu::make_penalty_dynamic_vector_function(pft::Penalty{0.0, 2.0});
    const auto tRosenbrock = pfitu::make_rosenbrock_dynamic_vector_function(pft::Rosenbrock{});
    const auto tComposition = core::compose(tRosenbrock, tPenalty);

    const auto tControl = linear_algebra::DynamicVector<double>{std::vector{1.0, 1.0}};
    const double tCompositionOfX = tComposition.f(tControl);
    EXPECT_EQ(tCompositionOfX, 0.0);

    const linear_algebra::DynamicVector<double> tDCompositionOfX = tComposition.df(tControl);
    EXPECT_EQ(tDCompositionOfX[0], 0.0);
    EXPECT_EQ(tDCompositionOfX[1], 0.0);
}
}  // namespace plato::integration_tests::serial
