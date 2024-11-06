#include <gtest/gtest.h>

#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/snopt/SNOPTConstraints.hpp"
#include "plato/third_party_integration/snopt/test_utilities/TestUtilities.hpp"
#include "plato/utilities/Enumerate.hpp"
#include "plato/utilities/IndexRange.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::third_party_integration::snopt::unittest
{
namespace
{
constexpr auto kScalarConstraintDimension = std::size_t{1U};

auto linear_function(const linear_algebra::DynamicVector<double>& aX) -> double { return 2.0 * aX[0] - aX[1]; }

auto affine_linear_function(const linear_algebra::DynamicVector<double>& aX) -> double
{
    return 2.0 * aX[0] - aX[1] - 1.0;
}

auto linear_function_gradient() -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>{2.0, -1.0};
}

const auto kLinearTestInterfaceFunction = plato::core::make_function_with_first_derivative(
    [](const linear_algebra::DynamicVector<double>& aX)
    { return test_utilities::vector_from_scalar(linear_function(aX)); },
    [](const linear_algebra::DynamicVector<double>&)
    { return test_utilities::jacobian_from_gradient(linear_function_gradient()); });

const auto kAffineLinearTestInterfaceFunction = plato::core::make_function_with_first_derivative(
    [](const linear_algebra::DynamicVector<double>& aX)
    { return test_utilities::vector_from_scalar(affine_linear_function(aX)); },
    [](const linear_algebra::DynamicVector<double>&)
    { return test_utilities::jacobian_from_gradient(linear_function_gradient()); });

const auto kArbitraryTestInterfaceFunction = plato::core::make_function_with_first_derivative(
    [](const linear_algebra::DynamicVector<double>&) { return test_utilities::vector_from_scalar(1.0); },
    [](const linear_algebra::DynamicVector<double>& aX) { return test_utilities::jacobian_from_gradient(aX); });

const auto kLinearConstraintTarget = std::vector{0.0};
const auto kNonlinearConstraintTarget = std::vector{2.0};

const auto kLinearInterfaceConstraint = InterfaceConstraintType{kLinearTestInterfaceFunction, kLinearConstraintTarget,
                                                                Linearity::kLinear, kScalarConstraintDimension};
const auto kAffineLinearInterfaceConstraint = InterfaceConstraintType{
    kAffineLinearTestInterfaceFunction, kLinearConstraintTarget, Linearity::kLinear, kScalarConstraintDimension};
const auto kNonlinearInterfaceConstraint = InterfaceConstraintType{
    kArbitraryTestInterfaceFunction, kNonlinearConstraintTarget, Linearity::kNonlinear, kScalarConstraintDimension};

const auto kInterfaceConstraints =
    std::vector{kLinearInterfaceConstraint, kNonlinearInterfaceConstraint, kAffineLinearInterfaceConstraint,
                kNonlinearInterfaceConstraint, kLinearInterfaceConstraint};

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

const auto kAffineLinearVectorConstraint = InterfaceConstraintType{
    kAffineLinearVectorTestFunction, kVectorConstraintTarget, Linearity::kLinear, kVectorConstraintDimension};
const auto kNonlinearVectorConstraint = InterfaceConstraintType{kArbitraryVectorTestFunction, kVectorConstraintTarget,
                                                                Linearity::kNonlinear, kVectorConstraintDimension};

const auto kVectorConstraints = std::vector{kAffineLinearVectorConstraint, kNonlinearVectorConstraint};

constexpr auto kNumberOfLinearConstraints = std::size_t{3};
constexpr auto kNumberOfNonlinearConstraints = std::size_t{2};
constexpr auto kNumberOfDesignVariables = std::size_t{2};

void check_scalar_vs_vector_constraint(const InterfaceConstraintType& aVectorConstraint,
                                       const ConstraintVectorType& aScalarConstraints,
                                       const linear_algebra::DynamicVector<double>& aDesignVariables,
                                       const plato::test_utilities::TestContext& aTestContext)
{
    ASSERT_EQ(aVectorConstraint.mConstraintDimension, aScalarConstraints.size()) << aTestContext;
    EXPECT_EQ(aVectorConstraint.mLinearity, aScalarConstraints.front().mLinearity) << aTestContext;
    EXPECT_EQ(aVectorConstraint.mConstraintType, aScalarConstraints.front().mConstraintType) << aTestContext;
    for (const auto& [aVectorTarget, tScalarConstraint] :
         utilities::Zip{aVectorConstraint.mTargets, aScalarConstraints})
    {
        EXPECT_EQ(aVectorTarget, tScalarConstraint.mTarget) << aTestContext;
    }

    // Constraint function evaluation
    const auto tExpectedConstraintEvaluation =
        aVectorConstraint.mFunction.template evaluate<core::evaluation::kFunction>(aDesignVariables);
    for (const auto& [tComponentIndex, tScalarConstraint] : utilities::enumerate(aScalarConstraints))
    {
        const auto tResultConstraintEvaluation =
            tScalarConstraint.mFunction.template evaluate<core::evaluation::kFunction>(aDesignVariables);
        EXPECT_EQ(tExpectedConstraintEvaluation[tComponentIndex], tResultConstraintEvaluation) << aTestContext;
    }

    // Constraint Jacobian evaluation
    const auto tConstraintJacobian =
        aVectorConstraint.mFunction.template evaluate<core::evaluation::kFirstDerivative>(aDesignVariables);
    for (const auto& [tComponentIndex, tScalarConstraint] : utilities::enumerate(aScalarConstraints))
    {
        auto tBasisVector = std::vector<double>(aScalarConstraints.size(), 0.0);
        tBasisVector[tComponentIndex] = 1.0;
        const auto tExpectedConstraintJacobianEvaluationRow =
            linear_algebra::DynamicVector<double>(std::move(tBasisVector)) * tConstraintJacobian;

        const auto tResultConstraintGradient =
            tScalarConstraint.mFunction.template evaluate<core::evaluation::kFirstDerivative>(aDesignVariables);

        EXPECT_EQ(tExpectedConstraintJacobianEvaluationRow.stdVector(), tResultConstraintGradient.stdVector())
            << aTestContext;
    }
}
}  // namespace

TEST(Constraints, ConstructionPartitioning)
{
    auto tSNOPTConstraints =
        SNOPTConstraints{InterfaceConstraintVectorType{kInterfaceConstraints}, kNumberOfDesignVariables};
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
    const auto kLinearTestFunction = plato::core::make_function_with_first_derivative(
        [](const linear_algebra::DynamicVector<double>& aX) { return linear_function(aX); },
        [](const linear_algebra::DynamicVector<double>&) { return linear_function_gradient(); });

    const auto kAffineLinearTestFunction = plato::core::make_function_with_first_derivative(
        [](const linear_algebra::DynamicVector<double>& aX) { return affine_linear_function(aX); },
        [](const linear_algebra::DynamicVector<double>&) { return linear_function_gradient(); });

    const auto kArbitraryTestFunction = plato::core::make_function_with_first_derivative(
        [](const linear_algebra::DynamicVector<double>&) { return 1.0; },
        [](const linear_algebra::DynamicVector<double>& aX) { return aX; });

    const auto kLinearConstraint =
        ConstraintData{kLinearTestFunction, kLinearConstraintTarget.front(), Linearity::kLinear};
    const auto kAffineConstraint =
        ConstraintData{kAffineLinearTestFunction, kLinearConstraintTarget.front(), Linearity::kLinear};
    const auto kNonlinearConstraint =
        ConstraintData{kArbitraryTestFunction, kNonlinearConstraintTarget.front(), Linearity::kNonlinear};

    const auto tConstraintsWithOffsets = std::vector{kLinearConstraint, kNonlinearConstraint, kAffineConstraint,
                                                     kNonlinearConstraint, kLinearConstraint};

    const auto tResultConstraints =
        constraints_with_affine_offset_removed(ConstraintVectorType{tConstraintsWithOffsets}, kNumberOfDesignVariables);

    const auto tAffineLinearConstraintTarget = 1.0;

    EXPECT_EQ(tResultConstraints.at(0).mTarget, kLinearConstraintTarget.front());
    EXPECT_EQ(tResultConstraints.at(1).mTarget, kNonlinearConstraintTarget.front());
    EXPECT_EQ(tResultConstraints.at(2).mTarget, tAffineLinearConstraintTarget);
    EXPECT_EQ(tResultConstraints.at(3).mTarget, kNonlinearConstraintTarget.front());
    EXPECT_EQ(tResultConstraints.at(4).mTarget, kLinearConstraintTarget.front());
}

TEST(Constraints, NumberOfConstraints)
{
    const auto tSNOPTConstraints =
        SNOPTConstraints{InterfaceConstraintVectorType{kInterfaceConstraints}, kNumberOfDesignVariables};

    EXPECT_EQ(tSNOPTConstraints.numberOfLinearConstraints(), kNumberOfLinearConstraints);
    EXPECT_EQ(tSNOPTConstraints.numberOfNonlinearConstraints(), kNumberOfNonlinearConstraints);
}

TEST(Constraints, LinearConstraintIterators)
{
    auto tSNOPTConstraints =
        SNOPTConstraints{InterfaceConstraintVectorType{kInterfaceConstraints}, kNumberOfDesignVariables};

    EXPECT_EQ(std::distance(tSNOPTConstraints.linearConstraintsBegin(), tSNOPTConstraints.linearConstraintsEnd()),
              kNumberOfLinearConstraints);
    // Check that they're all linear
    std::for_each(tSNOPTConstraints.linearConstraintsBegin(), tSNOPTConstraints.linearConstraintsEnd(),
                  [](const auto& tLinearConstraint) { EXPECT_EQ(tLinearConstraint.mLinearity, Linearity::kLinear); });
}

TEST(Constraints, NonlinearConstraintIterators)
{
    auto tSNOPTConstraints =
        SNOPTConstraints{InterfaceConstraintVectorType{kInterfaceConstraints}, kNumberOfDesignVariables};

    EXPECT_EQ(std::distance(tSNOPTConstraints.nonlinearConstraintsBegin(), tSNOPTConstraints.nonlinearConstraintsEnd()),
              kNumberOfNonlinearConstraints);
    // Check that they're all nonlinear
    std::for_each(tSNOPTConstraints.nonlinearConstraintsBegin(), tSNOPTConstraints.nonlinearConstraintsEnd(),
                  [](const auto& tLinearConstraint)
                  { EXPECT_EQ(tLinearConstraint.mLinearity, Linearity::kNonlinear); });
}

TEST(Constraints, ConstraintBoundsEquality)
{
    const auto tSNOPTConstraints =
        SNOPTConstraints{InterfaceConstraintVectorType{kInterfaceConstraints}, kNumberOfDesignVariables};
    const auto [tLowerBounds, tUpperBounds] = constraint_bounds(tSNOPTConstraints);

    ASSERT_EQ(tLowerBounds.size(), tSNOPTConstraints.constraints().size());
    ASSERT_EQ(tUpperBounds.size(), tSNOPTConstraints.constraints().size());

    for (const auto tIndex : utilities::IndexRange{tSNOPTConstraints.constraints().size()})
    {
        if (tIndex < kNumberOfNonlinearConstraints)
        {
            EXPECT_EQ(tLowerBounds.at(tIndex), kNonlinearConstraintTarget.front());
            EXPECT_EQ(tUpperBounds.at(tIndex), kNonlinearConstraintTarget.front());
        }
        else
        {
            const auto tZero = ConstraintFunctionArgument(kNumberOfDesignVariables, 0.0);
            const auto tFOfZero =
                tSNOPTConstraints.constraints().at(tIndex).mFunction.template evaluate<core::evaluation::kFunction>(
                    tZero);
            EXPECT_EQ(tLowerBounds.at(tIndex), -tFOfZero);
            EXPECT_EQ(tUpperBounds.at(tIndex), -tFOfZero);
        }
    }
}

TEST(Constraints, ConstraintBoundsInequality)
{
    auto tInterfaceConstraints =
        InterfaceConstraintVectorType{kNonlinearInterfaceConstraint, kLinearInterfaceConstraint};
    tInterfaceConstraints.front().mConstraintType = ConstraintType::kGreaterThan;
    tInterfaceConstraints.back().mConstraintType = ConstraintType::kLesserThan;

    const auto tSNOPTConstraints =
        SNOPTConstraints{InterfaceConstraintVectorType{tInterfaceConstraints}, kNumberOfDesignVariables};

    const auto [tLowerBounds, tUpperBounds] = constraint_bounds(tSNOPTConstraints);

    ASSERT_EQ(tLowerBounds.size(), tInterfaceConstraints.size());
    ASSERT_EQ(tUpperBounds.size(), tInterfaceConstraints.size());

    EXPECT_EQ(tLowerBounds.front(), tInterfaceConstraints.front().mTargets.front());
    EXPECT_EQ(tLowerBounds.back(), -kSNOPTUnbounded);

    EXPECT_EQ(tUpperBounds.front(), kSNOPTUnbounded);
    EXPECT_EQ(tUpperBounds.back(), tInterfaceConstraints.back().mTargets.front());
}

TEST(Constraints, VectorConstraintBoundsEquality)
{
    const auto tSNOPTConstraints =
        SNOPTConstraints{InterfaceConstraintVectorType{kVectorConstraints}, kNumberOfDesignVariables};
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
    const auto tSNOPTConstraints =
        SNOPTConstraints{InterfaceConstraintVectorType{kInterfaceConstraints}, kNumberOfDesignVariables};
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

TEST(Constraints, ConvertToScalarAllScalar)
{
    const auto tExpandedConstraints =
        constraints_with_vectors_expanded(InterfaceConstraintVectorType{kInterfaceConstraints});

    ASSERT_EQ(tExpandedConstraints.size(), kInterfaceConstraints.size());

    const auto tDesignVariables = linear_algebra::DynamicVector{-1.0, 2.0};
    for (const auto& [tResultConstraint, tOriginalConstraint] :
         utilities::Zip{tExpandedConstraints, kInterfaceConstraints})
    {
        check_scalar_vs_vector_constraint(tOriginalConstraint, {tResultConstraint}, tDesignVariables,
                                          TEST_CONTEXT("Multiple scalar constraints"));
    }
}

TEST(Constraints, ConvertToScalarAllVector)
{
    const auto tExpandedConstraints =
        constraints_with_vectors_expanded(InterfaceConstraintVectorType{kVectorConstraints});

    ASSERT_EQ(tExpandedConstraints.size(), kVectorConstraintDimension * kVectorConstraints.size());

    const auto tDesignVariables = linear_algebra::DynamicVector{2.0, -1.0};
    auto tResultConstraintIterator = tExpandedConstraints.begin();
    for (const auto& tOriginalConstraint : kVectorConstraints)
    {
        check_scalar_vs_vector_constraint(tOriginalConstraint,
                                          {tResultConstraintIterator, tResultConstraintIterator + 2U}, tDesignVariables,
                                          TEST_CONTEXT("Multiple vector constraints"));
        std::advance(tResultConstraintIterator, 2U);
    }
}

TEST(Constraints, ConvertToScalarSingleScalarConstraint)
{
    const auto tScalarConstraints =
        detail::constraint_with_vectors_expanded(InterfaceConstraintType{kLinearInterfaceConstraint});

    const auto tDesignVariables = linear_algebra::DynamicVector{10.0, 11.0};
    check_scalar_vs_vector_constraint(kLinearInterfaceConstraint, tScalarConstraints, tDesignVariables,
                                      TEST_CONTEXT("Single scalar constraint"));
}

TEST(Constraints, ConvertToScalarSingleVectorConstraint)
{
    auto tInterfaceConstraint = InterfaceConstraintType{kAffineLinearVectorConstraint};
    tInterfaceConstraint.mConstraintType = ConstraintType::kGreaterThan;
    const auto tScalarConstraints = detail::constraint_with_vectors_expanded(tInterfaceConstraint);

    const auto tDesignVariables = linear_algebra::DynamicVector{-10.0, -11.0};
    check_scalar_vs_vector_constraint(tInterfaceConstraint, tScalarConstraints, tDesignVariables,
                                      TEST_CONTEXT("Single vector constraint"));
}

TEST(Constraints, TotalNumberOfScalarConstraints)
{
    const auto tResultNumberOfConstraints =
        detail::total_number_of_scalar_constraints(InterfaceConstraintVectorType{kVectorConstraints});
    const auto tExpectedNumberOfConstraints =
        kVectorConstraints.front().mConstraintDimension + kVectorConstraints.back().mConstraintDimension;
    EXPECT_EQ(tResultNumberOfConstraints, tExpectedNumberOfConstraints);
}

}  // namespace plato::third_party_integration::snopt::unittest
