#include <gtest/gtest.h>

#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/third_party_integration/snopt/SNOPTConstraints.hpp"
#include "plato/third_party_integration/snopt/test_utilities/TestUtilities.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::third_party_integration::snopt::unittest
{
namespace
{
constexpr auto kScalarConstraintDimension = std::size_t{1U};

const auto kLinearTestFunction = plato::core::make_function_with_first_derivative(
    [](const linear_algebra::DynamicVector<double>& aX)
    { return test_utilities::vector_from_scalar(2.0 * aX[0] - aX[1]); },
    [](const linear_algebra::DynamicVector<double>&) {
        return test_utilities::jacobian_from_gradient(linear_algebra::DynamicVector<double>{2.0, -1.0});
    });

const auto kAffineLinearTestFunction = plato::core::make_function_with_first_derivative(
    [](const linear_algebra::DynamicVector<double>& aX)
    { return test_utilities::vector_from_scalar(2.0 * aX[0] - aX[1] - 1.0); },
    [](const linear_algebra::DynamicVector<double>&) {
        return test_utilities::jacobian_from_gradient(linear_algebra::DynamicVector<double>{2.0, -1.0});
    });

const auto kArbitraryTestFunction = plato::core::make_function_with_first_derivative(
    [](const linear_algebra::DynamicVector<double>&) { return test_utilities::vector_from_scalar(1.0); },
    [](const linear_algebra::DynamicVector<double>& aX) { return test_utilities::jacobian_from_gradient(aX); });

const auto kLinearConstraintTarget = std::vector{0.0};
const auto kAffineLinearConstraintTarget = std::vector{1.0};
const auto kNonlinearConstraintTarget = std::vector{2.0};

const auto kLinearConstraint =
    ConstraintType{kLinearTestFunction, kLinearConstraintTarget, Linearity::kLinear, kScalarConstraintDimension};
const auto kAffineLinearConstraint =
    ConstraintType{kAffineLinearTestFunction, kLinearConstraintTarget, Linearity::kLinear, kScalarConstraintDimension};
const auto kNonlinearConstraint = ConstraintType{kArbitraryTestFunction, kNonlinearConstraintTarget,
                                                 Linearity::kNonlinear, kScalarConstraintDimension};

const auto kConstraints = std::vector{kLinearConstraint, kNonlinearConstraint, kAffineLinearConstraint,
                                      kNonlinearConstraint, kLinearConstraint};

const auto kAffineLinearVectorTestFunction = plato::core::make_function_with_first_derivative(
    [](const linear_algebra::DynamicVector<double>& aX) {
        return linear_algebra::DynamicVector<double>{2.0 * aX[0] - aX[1], -2.0 * aX[0] + 3.0 * aX[1] + 2.0};
    },
    [](const linear_algebra::DynamicVector<double>&)
    {
        return linear_algebra::JacobianMultiplier{[](const linear_algebra::DynamicVector<double>& aVector)
                                                  {
                                                      return linear_algebra::DynamicVector<double>{
                                                          2.0 * aVector[0] - 2.0 * aVector[1],
                                                          -1.0 * aVector[0] + 3.0 * aVector[1]};
                                                  }};
    });

const auto kArbitraryVectorTestFunction = plato::core::make_function_with_first_derivative(
    [](const linear_algebra::DynamicVector<double>&) {
        return linear_algebra::DynamicVector<double>{1.0, 2.0};
    },
    [](const linear_algebra::DynamicVector<double>&)
    {
        return linear_algebra::JacobianMultiplier{[](const linear_algebra::DynamicVector<double>&) {
            return linear_algebra::DynamicVector<double>{-1.0, -2.0};
        }};
    });

const auto kVectorConstraintDimension = std::size_t{2U};

const auto kVectorConstraintTarget = std::vector{0.0, 2.0};

const auto kAffineLinearVectorConstraint = ConstraintType{kAffineLinearVectorTestFunction, kVectorConstraintTarget,
                                                          Linearity::kLinear, kVectorConstraintDimension};
const auto kNonlinearVectorConstraint = ConstraintType{kArbitraryVectorTestFunction, kVectorConstraintTarget,
                                                       Linearity::kNonlinear, kVectorConstraintDimension};

const auto kVectorConstraints = std::vector{kAffineLinearVectorConstraint, kNonlinearVectorConstraint};

constexpr auto kNumberOfLinearConstraints = std::size_t{3};
constexpr auto kNumberOfNonlinearConstraints = std::size_t{2};
constexpr auto kNumberOfDesignVariables = std::size_t{2};
}  // namespace

TEST(Constraints, ConstructionPartitioning)
{
    auto tSNOPTConstraints = SNOPTConstraints{ConstraintVectorType{kConstraints}, kNumberOfDesignVariables};
    const auto& tConstraints = tSNOPTConstraints.constraints();

    ASSERT_EQ(tConstraints.size(), 5U);
    // Expect that the first two are non-linear
    EXPECT_EQ(tConstraints.at(0).mLinearity, Linearity::kNonlinear);
    EXPECT_EQ(tConstraints.at(1).mLinearity, Linearity::kNonlinear);
    // Expect that the last three are linear
    EXPECT_EQ(tConstraints.at(2).mLinearity, Linearity::kLinear);
    EXPECT_EQ(tConstraints.at(3).mLinearity, Linearity::kLinear);
    EXPECT_EQ(tConstraints.at(4).mLinearity, Linearity::kLinear);
}

TEST(Constraints, RemoveAffineOffset)
{
    auto tConstraints =
        constraints_with_affine_offset_removed(ConstraintVectorType{kConstraints}, kNumberOfDesignVariables);

    EXPECT_EQ(tConstraints.at(0).mTargets, kLinearConstraintTarget);
    EXPECT_EQ(tConstraints.at(1).mTargets, kNonlinearConstraintTarget);
    EXPECT_EQ(tConstraints.at(2).mTargets, kAffineLinearConstraintTarget);
    EXPECT_EQ(tConstraints.at(3).mTargets, kNonlinearConstraintTarget);
    EXPECT_EQ(tConstraints.at(4).mTargets, kLinearConstraintTarget);
}

TEST(Constraints, RemoveAffineOffsetVector)
{
    auto tConstraints =
        constraints_with_affine_offset_removed(ConstraintVectorType{kVectorConstraints}, kNumberOfDesignVariables);

    const auto tExpectedVectorConstraintsAfterOffsetRemoval = std::vector{0.0, 0.0};
    EXPECT_EQ(tConstraints.at(0).mTargets, tExpectedVectorConstraintsAfterOffsetRemoval);
    EXPECT_EQ(tConstraints.at(1).mTargets, kVectorConstraintTarget);
}

TEST(Constraints, NumberOfConstraints)
{
    const auto tSNOPTConstraints = SNOPTConstraints{ConstraintVectorType{kConstraints}, kNumberOfDesignVariables};

    EXPECT_EQ(tSNOPTConstraints.numberOfLinearConstraints(), kNumberOfLinearConstraints);
    EXPECT_EQ(tSNOPTConstraints.numberOfNonlinearConstraints(), kNumberOfNonlinearConstraints);
    EXPECT_EQ(tSNOPTConstraints.linearConstraintDimension(), kNumberOfLinearConstraints);
    EXPECT_EQ(tSNOPTConstraints.nonlinearConstraintDimension(), kNumberOfNonlinearConstraints);
}

TEST(Constraints, ConstraintDimension)
{
    const auto tSNOPTConstraints = SNOPTConstraints{ConstraintVectorType{kVectorConstraints}, kNumberOfDesignVariables};

    constexpr auto tExpectedNumberOfLinearConstraints = 1U;
    EXPECT_EQ(tSNOPTConstraints.numberOfLinearConstraints(), tExpectedNumberOfLinearConstraints);
    constexpr auto tExpectedNumberOfNoninearConstraints = 1U;
    EXPECT_EQ(tSNOPTConstraints.numberOfNonlinearConstraints(), tExpectedNumberOfNoninearConstraints);

    constexpr auto tExpectedNonlinearConstraintDimension = 2U;
    EXPECT_EQ(tSNOPTConstraints.nonlinearConstraintDimension(), kNumberOfLinearConstraints);
    constexpr auto tExpectedLinearConstraintDimension = 2U;
    EXPECT_EQ(tSNOPTConstraints.linearConstraintDimension(), kNumberOfNonlinearConstraints);
}

TEST(Constraints, LinearConstraintIterators)
{
    auto tSNOPTConstraints = SNOPTConstraints{ConstraintVectorType{kConstraints}, kNumberOfDesignVariables};

    EXPECT_EQ(std::distance(tSNOPTConstraints.linearConstraintsBegin(), tSNOPTConstraints.linearConstraintsEnd()),
              kNumberOfLinearConstraints);
    // Check that they're all linear
    std::for_each(tSNOPTConstraints.linearConstraintsBegin(), tSNOPTConstraints.linearConstraintsEnd(),
                  [](const auto& tLinearConstraint) { EXPECT_EQ(tLinearConstraint.mLinearity, Linearity::kLinear); });
}

TEST(Constraints, NonlinearConstraintIterators)
{
    auto tSNOPTConstraints = SNOPTConstraints{ConstraintVectorType{kConstraints}, kNumberOfDesignVariables};

    EXPECT_EQ(std::distance(tSNOPTConstraints.nonlinearConstraintsBegin(), tSNOPTConstraints.nonlinearConstraintsEnd()),
              kNumberOfNonlinearConstraints);
    // Check that they're all nonlinear
    std::for_each(tSNOPTConstraints.nonlinearConstraintsBegin(), tSNOPTConstraints.nonlinearConstraintsEnd(),
                  [](const auto& tLinearConstraint)
                  { EXPECT_EQ(tLinearConstraint.mLinearity, Linearity::kNonlinear); });
}

TEST(Constraints, ConstraintBounds)
{
    const auto tSNOPTConstraints = SNOPTConstraints{ConstraintVectorType{kConstraints}, kNumberOfDesignVariables};
    const auto [tLowerBounds, tUpperBounds] = constraint_bounds(tSNOPTConstraints);

    ASSERT_EQ(tLowerBounds.size(), tSNOPTConstraints.constraints().size());
    ASSERT_EQ(tUpperBounds.size(), tSNOPTConstraints.constraints().size());

    for (const auto tIndex : utilities::IndexRange{tSNOPTConstraints.constraints().size()})
    {
        if (tIndex < kNumberOfNonlinearConstraints)
        {
            EXPECT_EQ(tLowerBounds.at(tIndex), kNonlinearConstraintTarget);
            EXPECT_EQ(tUpperBounds.at(tIndex), kNonlinearConstraintTarget);
        }
        else
        {
            const auto tZero = ConstraintFunctionArgument(kNumberOfDesignVariables, 0.0);
            const auto tFOfZero =
                tSNOPTConstraints.constraints().at(tIndex).mFunction.template evaluate<core::evaluation::kFunction>(
                    tZero);
            ASSERT_EQ(tFOfZero.size(), 1);
            EXPECT_EQ(tLowerBounds.at(tIndex), -tFOfZero[0]);
            EXPECT_EQ(tUpperBounds.at(tIndex), -tFOfZero[0]);
        }
    }
}

TEST(Constraints, VectorConstraintBounds)
{
    const auto tSNOPTConstraints = SNOPTConstraints{ConstraintVectorType{kVectorConstraints}, kNumberOfDesignVariables};
    const auto [tLowerBounds, tUpperBounds] =
        constraint_bounds(tSNOPTConstraints);  // Test constraint_bounds with a vector constraint

    const auto tVectorConstraintDimensions = kVectorConstraintTarget.size();
    const auto tExpectedNumberOfConstraintBounds = tVectorConstraintDimensions * kVectorConstraints.size();
    ASSERT_EQ(tExpectedNumberOfConstraintBounds, tLowerBounds.size());
    ASSERT_EQ(tExpectedNumberOfConstraintBounds, tUpperBounds.size());

    // Non-linear
    EXPECT_EQ(tLowerBounds.at(0), kVectorConstraintTarget.at(0));
    EXPECT_EQ(tUpperBounds.at(0), kVectorConstraintTarget.at(0));
    EXPECT_EQ(tLowerBounds.at(1), kVectorConstraintTarget.at(1));
    EXPECT_EQ(tUpperBounds.at(1), kVectorConstraintTarget.at(1));

    // Affine-linear
    const auto tExpectedVectorConstraintsAfterOffsetRemoval = std::vector{0.0, 0.0};
    EXPECT_EQ(tLowerBounds.at(2), tExpectedVectorConstraintsAfterOffsetRemoval.at(0));
    EXPECT_EQ(tUpperBounds.at(2), tExpectedVectorConstraintsAfterOffsetRemoval.at(0));
    EXPECT_EQ(tLowerBounds.at(3), tExpectedVectorConstraintsAfterOffsetRemoval.at(1));
    EXPECT_EQ(tUpperBounds.at(3), tExpectedVectorConstraintsAfterOffsetRemoval.at(1));
}

TEST(Constraints, ObjectiveAndConstraintBounds)
{
    const auto tSNOPTConstraints = SNOPTConstraints{ConstraintVectorType{kConstraints}, kNumberOfDesignVariables};
    const auto [tLowerBounds, tUpperBounds] = constraint_bounds_with_unbounded_objective(tSNOPTConstraints);

    constexpr auto tNumberOfObjectives = 1U;
    ASSERT_EQ(tLowerBounds.size(), tSNOPTConstraints.constraints().size() + tNumberOfObjectives);
    ASSERT_EQ(tUpperBounds.size(), tSNOPTConstraints.constraints().size() + tNumberOfObjectives);

    EXPECT_LE(tLowerBounds.front(), -kSNOPTUnbounded);
    EXPECT_GE(tUpperBounds.front(), kSNOPTUnbounded);

    const auto [tLowerBoundsConstraintsOnly, tUpperBoundsConstraintsOnly] = constraint_bounds(tSNOPTConstraints);
    for (const auto tIndex : utilities::IndexRange{tLowerBoundsConstraintsOnly.size()})
    {
        EXPECT_EQ(tLowerBounds.at(tIndex + tNumberOfObjectives), tLowerBoundsConstraintsOnly.at(tIndex));
        EXPECT_EQ(tUpperBounds.at(tIndex + tNumberOfObjectives), tUpperBoundsConstraintsOnly.at(tIndex));
    }
}

}  // namespace plato::third_party_integration::snopt::unittest
