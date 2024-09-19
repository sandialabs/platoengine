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
auto to_two_d(const linear_algebra::DynamicVector<double>& aDynamicVector)
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
    const auto tVectorbrock = detail::make_vector_function(tRosenbrock);

    const auto tTestPoint = linear_algebra::DynamicVector<double>({1, 2});
    const auto tScalarGold = tRosenbrock.f(tTestPoint);
    const auto tVectorResult = tVectorbrock.f(tTestPoint).stdVector();

    ASSERT_EQ(tVectorResult.size(), 1u);
    EXPECT_EQ(tVectorResult[0], tScalarGold);

    const auto tTestDirection = linear_algebra::DynamicVector<double>({.6, 1.2});
    const auto tScalarDFGold = tRosenbrock.df(tTestPoint).dot(tTestDirection);

    const auto tVectorDFResult = tVectorbrock.df(tTestPoint).mJacobianTimesVectorFunction(tTestDirection).stdVector();
    ASSERT_EQ(tVectorDFResult.size(), 1u);
    EXPECT_EQ(tVectorDFResult[0], tScalarDFGold);
}

TEST(ConstraintAdaptor, MakeAdjointJacobianVectorFunction)
{
    auto tRosenbrock = make_rosenbrock_dynamic_vector_function();
    const auto tVectorbrock = detail::make_adjoint_jacobian_vector_function(tRosenbrock);

    const auto tTestPoint = linear_algebra::DynamicVector<double>({1, 2});
    const auto tTestDual = linear_algebra::DynamicVector<double>({.6});

    const auto tScalarDFGold = tRosenbrock.df(tTestPoint) * tTestDual.stdVector()[0];

    const auto tVectorDFResult = tVectorbrock.df(tTestPoint).mJacobianTimesVectorFunction(tTestDual).stdVector();
    ASSERT_EQ(tVectorDFResult.size(), 2u);
    EXPECT_EQ(tVectorDFResult[0], tScalarDFGold[0]);
    EXPECT_EQ(tVectorDFResult[1], tScalarDFGold[1]);
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
