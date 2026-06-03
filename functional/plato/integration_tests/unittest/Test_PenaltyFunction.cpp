#include <gtest/gtest.h>

#include <vector>

#include "plato/core/Compose.hpp"
#include "plato/core/Function.hpp"
#include "plato/integration_tests/utilities/DynamicVectorPenaltyFunction.hpp"
#include "plato/integration_tests/utilities/DynamicVectorRosenbrockFunction.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/Penalty.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::integration_tests::serial
{
TEST(PenaltyFunction, ValueAndJacobian)
{
    namespace pft = plato::test_utilities;
    namespace pfitu = plato::integration_tests::utilities;

    constexpr auto tXMin = pft::XMin{0.5e-2};
    constexpr auto tPower = pft::Exponent{2.0};
    const auto tPenalty = pfitu::make_penalty_dynamic_vector_function(pft::Penalty{tXMin, tPower});

    // Function and derivative at 1 and 0
    const auto tControl = linear_algebra::DynamicVector{1.0, 0.0};
    const linear_algebra::DynamicVector tFOfControl = tPenalty.evaluate<core::evaluation::kFunction>(tControl);
    EXPECT_EQ(tFOfControl[0], 1.0);
    EXPECT_EQ(tFOfControl[1], tXMin.mValue);

    const auto tDFOfControl = tPenalty.evaluate<core::evaluation::kFirstDerivative>(tControl);
    EXPECT_EQ(tDFOfControl.mJacobian(0, 0), tPower.mValue * (1.0 - tXMin.mValue));
    EXPECT_EQ(tDFOfControl.mJacobian(0, 1), 0.0);
    EXPECT_EQ(tDFOfControl.mJacobian(1, 0), 0.0);
    EXPECT_EQ(tDFOfControl.mJacobian(1, 1), 0.0);

    const linear_algebra::DynamicVector tColumn0 = tDFOfControl.column(0);
    EXPECT_EQ(tColumn0[0], tPower.mValue * (1.0 - tXMin.mValue));
    EXPECT_EQ(tColumn0[1], 0.0);

    const linear_algebra::DynamicVector tColumn1 = tDFOfControl.column(1);
    EXPECT_EQ(tColumn1[0], 0.0);
    EXPECT_EQ(tColumn1[1], 0.0);
}

TEST(PenaltyFunction, Multiplication)
{
    namespace pft = plato::test_utilities;

    const auto tX = linear_algebra::DynamicVector{1.0, 2.0};
    const auto tAParameter = utilities::DynamicVectorJacobian{pft::make_two_d_matrix(1.0, 2.0, 3.0, 4.0)};
    const auto tBParameter = tX * tAParameter;
    EXPECT_EQ(tBParameter[0], 7.0);
    EXPECT_EQ(tBParameter[1], 10.0);
}

TEST(PenaltyFunction, Composition)
{
    namespace pft = plato::test_utilities;
    namespace pfitu = plato::integration_tests::utilities;

    const auto tPenalty = pfitu::make_penalty_dynamic_vector_function(pft::Penalty{pft::XMin{0.0}, pft::Exponent{2.0}});
    const auto tRosenbrock = pfitu::make_rosenbrock_dynamic_vector_function(pft::Rosenbrock{});
    const auto tComposition = core::compose(tRosenbrock, tPenalty);

    const auto tControl = linear_algebra::DynamicVector<double>{std::vector{1.0, 1.0}};
    const double tCompositionOfX = tComposition.evaluate<core::evaluation::kFunction>(tControl);
    EXPECT_EQ(tCompositionOfX, 0.0);

    const linear_algebra::DynamicVector<double> tDCompositionOfX =
        tComposition.evaluate<core::evaluation::kFirstDerivative>(tControl);
    EXPECT_EQ(tDCompositionOfX[0], 0.0);
    EXPECT_EQ(tDCompositionOfX[1], 0.0);
}
}  // namespace plato::integration_tests::serial
