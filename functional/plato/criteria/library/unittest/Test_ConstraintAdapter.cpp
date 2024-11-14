#include <gtest/gtest.h>

#include "plato/core/test_utilities/Utilities.hpp"
#include "plato/criteria/library/ConstraintAdapter.hpp"
#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"
#include "plato/test_utilities/TwoDTestTypesToDynamicVector.hpp"

namespace plato::criteria::library::unittest
{
namespace
{

auto to_two_d(const linear_algebra::DynamicVector<double>& aDynamicVector) -> test_utilities::TwoDVector
{
    return test_utilities::TwoDVector{aDynamicVector[0], aDynamicVector[1]};
}

using LinearAlgebraFunction =
    core::Function<const linear_algebra::DynamicVector<double>&,
                   core::FunctionInfo<double, core::evaluation::kFunction>,
                   core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFirstDerivative>>;

using ConstraintDynamicVector = Constraint<const linear_algebra::DynamicVector<double>&>;

auto make_rosenbrock_function() -> LinearAlgebraFunction
{
    const auto tF = core::test_utilities::make_two_d_function(test_utilities::Rosenbrock{});
    return LinearAlgebraFunction{[tF](const linear_algebra::DynamicVector<double>& aDynamicVector)
                                 { return tF.evaluate<core::evaluation::kFunction>(to_two_d(aDynamicVector)); },
                                 [tF](const linear_algebra::DynamicVector<double>& aDynamicVector)
                                 {
                                     return test_utilities::to_dynamic_vector(
                                         tF.evaluate<core::evaluation::kFirstDerivative>(to_two_d(aDynamicVector)));
                                 }};
}

auto make_rosenbrock_constraint() -> Constraint<const linear_algebra::DynamicVector<double>&>
{
    constexpr double tValue = 0;
    constexpr bool tLinear = false;
    return ConstraintDynamicVector{"Rosenbrock", make_rosenbrock_function(), tValue, tLinear,
                                   ConstraintType::kLessThan};
}

}  // namespace

TEST(ConstraintAdaptor, MakeVectorFunction)
{
    auto tRosenbrock = make_rosenbrock_function();
    const auto tVectorRosenbrock =
        detail::make_vector_function<const linear_algebra::DynamicVector<double>&>(tRosenbrock);

    const auto tTestPoint = linear_algebra::DynamicVector<double>({1, 2});
    // Function evaluation
    {
        const auto tScalarGold = tRosenbrock.evaluate<core::evaluation::kFunction>(tTestPoint);
        const auto tVectorResult = tVectorRosenbrock.evaluate<core::evaluation::kFunction>(tTestPoint).stdVector();
        ASSERT_EQ(tVectorResult.size(), 1U);
        EXPECT_EQ(tVectorResult[0], tScalarGold);
    }
    // Gradient
    {
        constexpr auto tScaleFactor = double{2.0};
        const auto tTestDirection = linear_algebra::DynamicVector{tScaleFactor};
        const auto tExpected = tScaleFactor * tRosenbrock.evaluate<core::evaluation::kFirstDerivative>(tTestPoint);
        const auto tVectorDFResult =
            tTestDirection * tVectorRosenbrock.evaluate<core::evaluation::kFirstDerivative>(tTestPoint);
        ASSERT_EQ(tVectorDFResult.size(), 2U);
        EXPECT_EQ(tVectorDFResult.stdVector(), tExpected.stdVector());
    }
    // Adjoint gradient
    {
        const auto tTestDual = linear_algebra::DynamicVector{2.0, -4.0};
        const auto tExpected = tRosenbrock.evaluate<core::evaluation::kFirstDerivative>(tTestPoint).dot(tTestDual);
        const auto tVectorDFResult =
            tTestDual *
            tVectorRosenbrock.evaluate<core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>(tTestPoint);
        ASSERT_EQ(tVectorDFResult.size(), 1U);
        EXPECT_EQ(tVectorDFResult[0], tExpected);
    }
}

TEST(ConstraintAdaptor, MakeVectorConstraint)
{
    const auto tConstraint = make_rosenbrock_constraint();
    const auto tAdaptedConstraint = make_vector_constraint(tConstraint);

    EXPECT_EQ(tConstraint.mName, tAdaptedConstraint.mName);
    EXPECT_EQ(tConstraint.mConstraintTarget, tAdaptedConstraint.mConstraintTarget);
    EXPECT_EQ(tConstraint.mLinear, tAdaptedConstraint.mLinear);
    EXPECT_EQ(tConstraint.mConstraintType, tAdaptedConstraint.mConstraintType);

    const auto tTestPoint = linear_algebra::DynamicVector<double>({1, 2});
    const auto tScalarGold = tConstraint.mConstraintFunction.evaluate<core::evaluation::kFunction>(tTestPoint);
    const auto tVectorResult =
        tAdaptedConstraint.mConstraintFunction.evaluate<core::evaluation::kFunction>(tTestPoint).stdVector();

    ASSERT_EQ(tVectorResult.size(), 1u);
    EXPECT_EQ(tVectorResult[0], tScalarGold);

    const auto tVectorNoOp = make_vector_constraint(tAdaptedConstraint);
    EXPECT_EQ(tVectorNoOp.mName, tAdaptedConstraint.mName);
}

}  // namespace plato::criteria::library::unittest
