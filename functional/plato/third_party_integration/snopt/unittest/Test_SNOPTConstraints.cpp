#include <gtest/gtest.h>

#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/third_party_integration/snopt/SNOPTConstraints.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::third_party_integration::snopt::unittest
{
namespace
{
const auto kLinearTestFunction = plato::core::make_function_with_first_derivative(
    [](const linear_algebra::DynamicVector<double>& aX) { return 2.0 * aX[0] - aX[1]; },
    [](const linear_algebra::DynamicVector<double>&) {
        return linear_algebra::DynamicVector<double>{2.0, -1.0};
    });
const auto kAffineLinearTestFunction = plato::core::make_function_with_first_derivative(
    [](const linear_algebra::DynamicVector<double>& aX) { return 2.0 * aX[0] - aX[1] - 1.0; },
    [](const linear_algebra::DynamicVector<double>&) {
        return linear_algebra::DynamicVector<double>{2.0, -1.0};
    });
const auto kArbitraryTestFunction = plato::core::make_function_with_first_derivative(
    [](const linear_algebra::DynamicVector<double>&) { return 1.0; },
    [](const linear_algebra::DynamicVector<double>& aX) { return aX; });
constexpr auto kLinearConstraintTarget = 0.0;
constexpr auto kAffineLinearConstraintTarget = 1.0;
constexpr auto kNonlinearConstraintTarget = 2.0;

const auto kLinearConstraint = ConstraintType{kLinearTestFunction, kLinearConstraintTarget, Linearity::kLinear};
const auto kAffineLinearConstraint =
    ConstraintType{kAffineLinearTestFunction, kLinearConstraintTarget, Linearity::kLinear};
const auto kNonlinearConstraint =
    ConstraintType{kArbitraryTestFunction, kNonlinearConstraintTarget, Linearity::kNonlinear};

const auto kConstraints = std::vector{kLinearConstraint, kNonlinearConstraint, kAffineLinearConstraint,
                                      kNonlinearConstraint, kLinearConstraint};

constexpr auto kNumberOfLinearConstraints = std::size_t{3};
constexpr auto kNumberOfNonlinearConstraints = std::size_t{2};
constexpr auto kNumberofDesignVariables = std::size_t{2};
}  // namespace

TEST(Constraints, ConstructionPartitioning)
{
    auto tSNOPTConstraints = SNOPTConstraints{ConstraintVectorType{kConstraints}, kNumberofDesignVariables};
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
        constraints_with_affine_offset_removed(ConstraintVectorType{kConstraints}, kNumberofDesignVariables);

    EXPECT_EQ(tConstraints.at(0).mTarget, kLinearConstraintTarget);
    EXPECT_EQ(tConstraints.at(1).mTarget, kNonlinearConstraintTarget);
    EXPECT_EQ(tConstraints.at(2).mTarget, kAffineLinearConstraintTarget);
    EXPECT_EQ(tConstraints.at(3).mTarget, kNonlinearConstraintTarget);
    EXPECT_EQ(tConstraints.at(4).mTarget, kLinearConstraintTarget);
}

TEST(Constraints, NumberOfConstraints)
{
    const auto tSNOPTConstraints = SNOPTConstraints{ConstraintVectorType{kConstraints}, kNumberofDesignVariables};

    EXPECT_EQ(tSNOPTConstraints.numberOfLinearConstraints(), kNumberOfLinearConstraints);
    EXPECT_EQ(tSNOPTConstraints.numberOfNonlinearConstraints(), kNumberOfNonlinearConstraints);
}

TEST(Constraints, LinearConstraintIterators)
{
    auto tSNOPTConstraints = SNOPTConstraints{ConstraintVectorType{kConstraints}, kNumberofDesignVariables};

    EXPECT_EQ(std::distance(tSNOPTConstraints.linearConstraintsBegin(), tSNOPTConstraints.linearConstraintsEnd()),
              kNumberOfLinearConstraints);
    // Check that they're all linear
    std::for_each(tSNOPTConstraints.linearConstraintsBegin(), tSNOPTConstraints.linearConstraintsEnd(),
                  [](const auto& tLinearConstraint) { EXPECT_EQ(tLinearConstraint.mLinearity, Linearity::kLinear); });
}

TEST(Constraints, NonlinearConstraintIterators)
{
    auto tSNOPTConstraints = SNOPTConstraints{ConstraintVectorType{kConstraints}, kNumberofDesignVariables};

    EXPECT_EQ(std::distance(tSNOPTConstraints.nonlinearConstraintsBegin(), tSNOPTConstraints.nonlinearConstraintsEnd()),
              kNumberOfNonlinearConstraints);
    // Check that they're all nonlinear
    std::for_each(tSNOPTConstraints.nonlinearConstraintsBegin(), tSNOPTConstraints.nonlinearConstraintsEnd(),
                  [](const auto& tLinearConstraint)
                  { EXPECT_EQ(tLinearConstraint.mLinearity, Linearity::kNonlinear); });
}

TEST(Constraints, ConstraintBounds)
{
    const auto tSNOPTConstraints = SNOPTConstraints{ConstraintVectorType{kConstraints}, kNumberofDesignVariables};
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
            const auto tZero = ConstraintFunctionArgument(kNumberofDesignVariables, 0.0);
            const auto fOfZero = tSNOPTConstraints.constraints().at(tIndex).mFunction.f(tZero);
            EXPECT_EQ(tLowerBounds.at(tIndex), -fOfZero);
            EXPECT_EQ(tUpperBounds.at(tIndex), -fOfZero);
        }
    }
}

TEST(Constraints, ObjectiveAndConstraintBounds)
{
    const auto tSNOPTConstraints = SNOPTConstraints{ConstraintVectorType{kConstraints}, kNumberofDesignVariables};
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
