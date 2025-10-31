#include <gtest/gtest.h>

#include "plato/criteria/library/TargetOffsetFunction.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::criteria::library
{
namespace
{
void check_identity_jacobian(const TargetOffsetFunction& aTargetFunction,
                             const linear_algebra::DynamicVector<double>& aJacobianArgument,
                             const test_utilities::TestContext& aTestContext)
{
    const auto tRowVector = linear_algebra::DynamicVector{-1.0, 1.0};
    {
        const auto tResult =
            tRowVector * aTargetFunction.evaluate<core::evaluation::kFirstDerivative>(aJacobianArgument);
        EXPECT_EQ(tRowVector.stdVector(), tResult.stdVector()) << aTestContext;
    }
    {
        const auto tResult =
            tRowVector * aTargetFunction.evaluate<core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>(
                             aJacobianArgument);
        EXPECT_EQ(tRowVector.stdVector(), tResult.stdVector()) << aTestContext;
    }
}
}  // namespace

TEST(TargetOffsetFunction, EvaluateVectorTarget)
{
    const auto tTargets = linear_algebra::DynamicVector{-1.0, 0.0, 1.0};
    const auto tTargetOffset = make_target_offset_function(tTargets);

    const auto tResult = tTargetOffset.evaluate<core::evaluation::kFunction>(tTargets);
    const auto tExpected = tTargets + (-1.0 * tTargets);

    EXPECT_EQ(tExpected.stdVector(), tResult.stdVector());
}

TEST(TargetOffsetFunction, JacobianVectorTarget)
{
    const auto tTargets = linear_algebra::DynamicVector{0.0, 1.0};
    const auto tTargetOffset = make_target_offset_function(tTargets);

    check_identity_jacobian(tTargetOffset, tTargets, TEST_CONTEXT("Vector targets"));
}

TEST(TargetOffsetFunction, EvaluateScalarTarget)
{
    const auto tTarget = 10.0;
    const auto tTargetOffset = make_target_offset_function(tTarget);

    const auto tX = linear_algebra::DynamicVector{10.0, 0.0, -1.0};
    const auto tResult = tTargetOffset.evaluate<core::evaluation::kFunction>(tX);
    const auto tExpected = tX + tTarget * linear_algebra::DynamicVector(tX.size(), -1.0);

    EXPECT_EQ(tExpected.stdVector(), tResult.stdVector());
}

TEST(TargetOffsetFunction, JacobianScalarTarget)
{
    const auto tTarget = 1.0;
    const auto tTargetOffset = make_target_offset_function(tTarget);
    const auto tJacobianArgument = linear_algebra::DynamicVector{0.0, 0.0};

    check_identity_jacobian(tTargetOffset, tJacobianArgument, TEST_CONTEXT("Scalar target"));
}

}  // namespace plato::criteria::library
