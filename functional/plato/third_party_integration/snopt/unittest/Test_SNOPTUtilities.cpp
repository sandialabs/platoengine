#include <gtest/gtest.h>

#include "plato/third_party_integration/snopt/DataSingleton.hpp"
#include "plato/third_party_integration/snopt/SNOPTUtilities.hpp"
#include "plato/third_party_integration/snopt/test_utilities/TestUtilities.hpp"
#include "plato/utilities/Enumerate.hpp"

namespace plato::third_party_integration::snopt::unittest
{
namespace
{
constexpr auto kNumberOfObjectives = std::size_t{1};

const auto kXSquaredFunction = [](const linear_algebra::DynamicVector<double>& aX) { return aX.dot(aX); };
const auto kXSquaredGradientFunction = [](const linear_algebra::DynamicVector<double>& aX) { return 2.0 * aX; };
const auto kXCubedFunction = [](const linear_algebra::DynamicVector<double>& aX)
{
    const auto tValue = aX.stdVector().front();
    return tValue * tValue * tValue;
};
const auto kXCubedGradientFunction = [](const linear_algebra::DynamicVector<double>& aX) { return 3.0 * aX; };

struct TestTag
{
};

const auto kXSquaredConstraint =
    ConstraintData{/*.mFunction=*/
                   core::make_function_with_first_derivative(kXSquaredFunction, kXSquaredGradientFunction),
                   /*.mTarget=*/0.0, /*.mLinearity=*/Linearity::kNonlinear};
const auto kXCubedConstraint =
    ConstraintData{/*.mFunction=*/
                   core::make_function_with_first_derivative(kXCubedFunction, kXCubedGradientFunction),
                   /*.mTarget=*/0.0, /*.mLinearity=*/Linearity::kNonlinear};

template <typename EvaluationFunction, typename GradientFunction>
auto test_linear_interface_constraint(EvaluationFunction aEvaluationFunction, GradientFunction aGradientFunction)
    -> InterfaceConstraintType
{
    return InterfaceConstraintType{
        /*.mFunction=*/core::make_function_with_first_derivative(aEvaluationFunction, aGradientFunction),
        /*.mTargets=*/{0.0},
        /*.mLinearity=*/Linearity::kLinear,
        /*.mConstraintDimension=*/1U};
}

template <typename EvaluationFunction, typename GradientFunction>
auto test_nonlinear_interface_constraint(EvaluationFunction aEvaluationFunction, GradientFunction aGradientFunction)
    -> InterfaceConstraintType
{
    return InterfaceConstraintType{
        /*.mFunction=*/core::make_function_with_first_derivative(aEvaluationFunction, aGradientFunction),
        /*.mTarget=s*/ {0.0},
        /*.mLinearity=*/Linearity::kNonlinear,
        /*.mConstraintDimension=*/1U};
}

template <typename EvaluationFunction, typename GradientFunction>
auto test_linear_constraint(EvaluationFunction aEvaluationFunction, GradientFunction aGradientFunction)
    -> ConstraintData
{
    return ConstraintData{
        /*.mFunction=*/core::make_function_with_first_derivative(aEvaluationFunction, aGradientFunction),
        /*.mTarget=*/0.0,
        /*.mLinearity=*/Linearity::kLinear};
}

template <typename EvaluationFunction, typename GradientFunction>
auto test_nonlinear_constraint(EvaluationFunction aEvaluationFunction, GradientFunction aGradientFunction)
    -> ConstraintData
{
    return ConstraintData{
        /*.mFunction=*/core::make_function_with_first_derivative(aEvaluationFunction, aGradientFunction),
        /*.mTarget=*/0.0,
        /*.mLinearity=*/Linearity::kNonlinear};
}

}  // namespace

TEST(SNOPTUtilities, LinearConstraintJacobians)
{
    using Row = typename SparseMatrixBuilder<IndexType, double>::Row;
    using Column = typename SparseMatrixBuilder<IndexType, double>::Column;

    constexpr auto tNumberOfDesignVariables = std::size_t{3};
    const auto tObjective = [](const linear_algebra::DynamicVector<double>&)
    { return test_utilities::vector_from_scalar(0.0); };
    const auto tGradient1 = [](const linear_algebra::DynamicVector<double>&) {
        return test_utilities::jacobian_from_gradient(linear_algebra::DynamicVector<double>{1.0, 0.0, 2.0});
    };
    const auto tGradient2 = [](const linear_algebra::DynamicVector<double>&) {
        return test_utilities::jacobian_from_gradient(linear_algebra::DynamicVector<double>{0.0, -1.0, 0.0});
    };
    auto tConstraintFunctions =
        InterfaceConstraintVectorType{test_linear_interface_constraint(tObjective, tGradient1),
                                      test_nonlinear_interface_constraint(tObjective, tGradient2),
                                      test_linear_interface_constraint(tObjective, tGradient2)};

    const auto tSparseMatrix = linear_constraint_jacobian_sparse_matrix(
        SNOPTConstraints{std::move(tConstraintFunctions), tNumberOfDesignVariables}, tNumberOfDesignVariables);
    constexpr auto tExpectedNumberOfNonZero = std::size_t{3};
    ASSERT_EQ(tSparseMatrix.size(), tExpectedNumberOfNonZero);

    const auto tExpectedTriples = std::vector<test_utilities::Triple<int, double>>{
        {Row{0}, Column{0}, 1.0}, {Row{0}, Column{2}, 2.0}, {Row{1}, Column{1}, -1.0}};

    for (const auto [tEntryIndex, tExpectedTriple] : utilities::enumerate(tExpectedTriples))
    {
        test_utilities::check_triple<int, double>(tExpectedTriple, tSparseMatrix.triple(tEntryIndex),
                                                  TEST_CONTEXT("Triples"));
    }
}

TEST(SNOPTUtilities, EvaluateObjective)
{
    DataSingleton<ObjectiveType, TestTag>::instance().data() =
        core::make_function_with_first_derivative(kXSquaredFunction, kXSquaredGradientFunction);

    auto tResult = std::vector<double>(kNumberOfObjectives);
    constexpr auto tX = 0.5;
    constexpr auto tNumberOfConstraints = ConstraintSizeType{0};
    evaluateObjective<TestTag>(linear_algebra::DynamicVector{tX},
                               ObjectiveConstraintArrayView{tResult.data(), tNumberOfConstraints});

    constexpr auto tExpected = 0.25;
    EXPECT_EQ(tResult.front(), tExpected);
}

TEST(SNOPTUtilities, EvaluateObjectiveGradient)
{
    DataSingleton<ObjectiveType, TestTag>::instance().data() =
        core::make_function_with_first_derivative(kXSquaredFunction, kXSquaredGradientFunction);

    constexpr auto tNumberOfConstraints = ConstraintSizeType{0};
    constexpr auto tNumberOfDesignVariables = DesignVariableSizeType{2};
    auto tResult = std::vector<double>(tNumberOfDesignVariables.mValue);
    const auto tX = linear_algebra::DynamicVector{0.5, 2.0};
    evaluateObjectiveGradient<TestTag>(
        tX, ObjectiveConstraintGradientArrayView{tResult.data(), tNumberOfConstraints, tNumberOfDesignVariables});

    const auto tExpected = std::vector{1.0, 4.0};
    EXPECT_EQ(tResult, tExpected);
}

TEST(SNOPTUtilities, EvaluateConstraints)
{
    DataSingleton<ConstraintVectorType, TestTag>::instance().data() = {kXSquaredConstraint, kXCubedConstraint};

    constexpr auto tNumberOfConstraints = ConstraintSizeType{2};
    auto tResult = std::vector<double>(kNumberOfObjectives + tNumberOfConstraints.mValue);
    constexpr auto tX = 0.5;
    evaluateConstraints<TestTag>(linear_algebra::DynamicVector{tX},
                                 ObjectiveConstraintArrayView{tResult.data(), tNumberOfConstraints});

    constexpr auto tExpected0 = 0.25;
    constexpr auto tExpected1 = 0.125;
    constexpr auto tFirstConstraintIndex = std::size_t{1};
    EXPECT_EQ(tResult.at(tFirstConstraintIndex), tExpected0);
    EXPECT_EQ(tResult.back(), tExpected1);
}

TEST(SNOPTUtilities, EvaluateConstraintGradients)
{
    const auto tObjective = [](const linear_algebra::DynamicVector<double>&) { return 0.0; };
    const auto tXTimesTwo = [](const linear_algebra::DynamicVector<double>& aX) { return 2.0 * aX; };
    const auto tXTimesThree = [](const linear_algebra::DynamicVector<double>& aX) { return 3.0 * aX; };

    DataSingleton<ConstraintVectorType, TestTag>::instance().data() = {
        test_linear_constraint(tObjective, tXTimesTwo), test_linear_constraint(tObjective, tXTimesThree)};

    constexpr auto tNumberOfConstraints = ConstraintSizeType{2};
    constexpr auto tNumberOfDesignVariables = DesignVariableSizeType{2};
    constexpr auto tArraySize = (kNumberOfObjectives + tNumberOfConstraints.mValue) * tNumberOfDesignVariables.mValue;
    auto tResult = std::vector<double>(tArraySize);

    const auto tX = linear_algebra::DynamicVector{-1.0, 10.0};

    evaluateConstraintGradient<TestTag>(
        tX, ObjectiveConstraintGradientArrayView{tResult.data(), tNumberOfConstraints, tNumberOfDesignVariables});

    const auto tExpected = std::vector{-2.0, 20.0, -3.0, 30.0};
    auto tConstraintsResult = std::vector<double>(2 * tNumberOfDesignVariables.mValue);
    std::copy(std::next(tResult.begin(), tNumberOfDesignVariables.mValue), tResult.end(), tConstraintsResult.begin());

    EXPECT_EQ(tExpected, tConstraintsResult);
}

}  // namespace plato::third_party_integration::snopt::unittest
