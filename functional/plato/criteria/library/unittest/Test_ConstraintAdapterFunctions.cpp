#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintAdapterFunctions.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::criteria::library::unittest
{
namespace
{
void check_identity_jacobian(const ConstraintAdapterFunction& aTargetFunction,
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

TEST(ConstraintAdapterFunctions, VectorSubsetFunctionEvaluation)
{
    const auto tIndices = std::set<std::size_t>{1U, 3U};
    const auto tSubsetFunction = make_vector_subset_function(tIndices);

    const auto tInput = linear_algebra::DynamicVector<double>{0.0, 1.0, 2.0, 3.0};
    const auto tResult = tSubsetFunction.evaluate<core::evaluation::kFunction>(tInput);
    const auto tExpected = linear_algebra::DynamicVector<double>{1.0, 3.0};
    EXPECT_EQ(tResult, tExpected);
}

TEST(ConstraintAdapterFunctions, VectorSubsetFunctionJacobian)
{
    const auto tIndices = std::set<std::size_t>{0U, 1U, 3U};
    const auto tSubsetFunction = make_vector_subset_function(tIndices);

    const auto tInput = linear_algebra::DynamicVector<double>{0.0, 0.0, 0.0, 0.0};
    const auto tRowVector = linear_algebra::DynamicVector<double>{10.0, 11.0, 12.0};
    const auto tResult = tRowVector * tSubsetFunction.evaluate<core::evaluation::kFirstDerivative>(tInput);
    // From matlab:
    // A = [1 0 0 0 ; 0 1 0 0 ; 0 0 0 1];
    // w = [10 11 12];
    // w * A
    const auto tExpected = linear_algebra::DynamicVector<double>{10.0, 11.0, 0.0, 12.0};
    EXPECT_EQ(tResult.stdVector(), tExpected.stdVector());
}

TEST(ConstraintAdapterFunctions, VectorSubsetFunctionAdjointJacobian)
{
    const auto tIndices = std::set<std::size_t>{2U};
    const auto tSubsetFunction = make_vector_subset_function(tIndices);

    const auto tInput = linear_algebra::DynamicVector<double>{0.0, 0.0, 0.0, 0.0};
    const auto tRowVector = linear_algebra::DynamicVector<double>{10.0, 11.0, 12.0, 13.0};
    const auto tResult =
        tRowVector *
        tSubsetFunction.evaluate<core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>(tInput);
    // From matlab:
    // A = [0 0 1 0];
    // w = [10 11 12 13];
    // w * A'
    const auto tExpected = linear_algebra::DynamicVector<double>{12.0};
    EXPECT_EQ(tResult, tExpected);
}

TEST(ConstraintAdapterFunctions, TargetOffsetFunctionEvaluateVectorTarget)
{
    const auto tTargets = linear_algebra::DynamicVector{-1.0, 0.0, 1.0};
    const auto tTargetOffset = make_target_offset_function(tTargets.stdVector());

    const auto tResult = tTargetOffset.evaluate<core::evaluation::kFunction>(tTargets);
    const auto tExpected = tTargets + (-1.0 * tTargets);

    EXPECT_EQ(tExpected.stdVector(), tResult.stdVector());
}

TEST(ConstraintAdapterFunctions, TargetOffsetFunctionJacobianVectorTarget)
{
    const auto tTargets = linear_algebra::DynamicVector{0.0, 1.0};
    const auto tTargetOffset = make_target_offset_function(tTargets.stdVector());

    check_identity_jacobian(tTargetOffset, tTargets, TEST_CONTEXT("Vector targets"));
}

TEST(ConstraintAdapterFunctions, TargetOffsetFunctionEvaluateScalarTarget)
{
    const auto tTarget = 10.0;
    const auto tTargetOffset = make_target_offset_function(tTarget);

    const auto tX = linear_algebra::DynamicVector{10.0, 0.0, -1.0};
    const auto tResult = tTargetOffset.evaluate<core::evaluation::kFunction>(tX);
    const auto tExpected = tX + tTarget * linear_algebra::DynamicVector(tX.size(), -1.0);

    EXPECT_EQ(tExpected.stdVector(), tResult.stdVector());
}

TEST(ConstraintAdapterFunctions, TargetOffsetFunctionJacobianScalarTarget)
{
    const auto tTarget = 1.0;
    const auto tTargetOffset = make_target_offset_function(tTarget);
    const auto tJacobianArgument = linear_algebra::DynamicVector{0.0, 0.0};

    check_identity_jacobian(tTargetOffset, tJacobianArgument, TEST_CONTEXT("Scalar target"));
}

}  // namespace plato::criteria::library::unittest
