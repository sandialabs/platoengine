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
    core::Function<double, linear_algebra::DynamicVector<double>, linear_algebra::DynamicVector<double>>;

auto make_rosenbrock_dynamic_vector_function() -> LinearAlgebraFunction
{
    const auto tF = core::test_utilities::make_rosenbrock_function(test_utilities::Rosenbrock{});
    return LinearAlgebraFunction{[tF](const auto aDynamicVector) { return tF.f(to_two_d(aDynamicVector)); },
                                 [tF](const auto aDynamicVector)
                                 { return test_utilities::to_dynamic_vector(tF.df(to_two_d(aDynamicVector))); }};
}

auto make_rosenbrock_constraint() -> Constraint<linear_algebra::DynamicVector<double>>
{
    const auto tDynamicVectorFunction = make_rosenbrock_dynamic_vector_function();
    constexpr double tValue = 0;
    constexpr bool tLinear = false;
    return Constraint<linear_algebra::DynamicVector<double>>{"Rosenbrock", tDynamicVectorFunction, tValue, tLinear,
                                                             ConstraintType::kLessThan};
}

}  // namespace

TEST(ConstraintAdaptor, MakeVectorFunction)
{
    auto tRosenbrock = make_rosenbrock_dynamic_vector_function();
    const auto tVectorRosenbrock = detail::make_vector_function(tRosenbrock);

    const auto tTestPoint = linear_algebra::DynamicVector<double>({1, 2});
    const auto tScalarGold = tRosenbrock.f(tTestPoint);
    const auto tVectorResult = tVectorRosenbrock.f(tTestPoint).stdVector();

    ASSERT_EQ(tVectorResult.size(), 1u);
    EXPECT_EQ(tVectorResult[0], tScalarGold);

    constexpr auto tScaleFactor = double{2.0};
    const auto tTestDirection = linear_algebra::DynamicVector{tScaleFactor};
    const auto tExpected = tScaleFactor * tRosenbrock.df(tTestPoint);

    const auto tVectorDFResult = tTestDirection * tVectorRosenbrock.df(tTestPoint);
    ASSERT_EQ(tVectorDFResult.size(), 2U);
    EXPECT_EQ(tVectorDFResult.stdVector(), tExpected.stdVector());
}

TEST(ConstraintAdaptor, MakeAdjointJacobianVectorFunction)
{
    auto tRosenbrock = make_rosenbrock_dynamic_vector_function();
    const auto tVectorRosenbrock = detail::make_adjoint_jacobian_vector_function(tRosenbrock);

    const auto tTestPoint = linear_algebra::DynamicVector{1.0, 2.0};
    const auto tTestDual = linear_algebra::DynamicVector{2.0, -4.0};

    const auto tExpected = tRosenbrock.df(tTestPoint).dot(tTestDual);

    const auto tVectorDFResult = tTestDual * tVectorRosenbrock.df(tTestPoint);
    ASSERT_EQ(tVectorDFResult.size(), 1U);
    EXPECT_EQ(tVectorDFResult[0], tExpected);
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
    const auto tScalarGold = tConstraint.mConstraintFunction.f(tTestPoint);
    const auto tVectorResult = tAdaptedConstraint.mFunctionWithDfAsJacobian.f(tTestPoint).stdVector();

    ASSERT_EQ(tVectorResult.size(), 1u);
    EXPECT_EQ(tVectorResult[0], tScalarGold);

    const auto tVectorNoOp = make_vector_constraint(tAdaptedConstraint);
    EXPECT_EQ(tVectorNoOp.mName, tAdaptedConstraint.mName);
}

}  // namespace plato::criteria::library::unittest
