#include <gtest/gtest.h>

#include <numeric>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/snopt/ObjectiveConstraintGradientArrayView.hpp"

namespace plato::third_party_integration::snopt::unittest
{
namespace
{
constexpr auto kNumberOfObjectives = std::size_t{1};

auto array_size(const ConstraintSizeType aNumberOfConstraints, const DesignVariableSizeType aNumberOfDesignVariables)
{
    return (aNumberOfConstraints.mValue + kNumberOfObjectives) * aNumberOfDesignVariables.mValue;
}

auto test_gradient_array(ConstraintSizeType aNumberOfConstraints, DesignVariableSizeType aNumberOfDesignVariables)
    -> std::vector<double>
{
    auto tVector =
        std::vector<double>((aNumberOfConstraints.mValue + kNumberOfObjectives) * aNumberOfDesignVariables.mValue);
    std::iota(tVector.begin(), tVector.end(), 0.0);
    return tVector;
}

template <typename T>
void check_gradient(const SNOPTArray<T> aGradientView,
                    const DesignVariableSizeType aNumberOfDesignVariables,
                    const double aStartingValue,
                    const test_utilities::TestContext& aTestContext)
{
    auto tExpectedObjective = std::vector<double>(aNumberOfDesignVariables.mValue);
    std::iota(tExpectedObjective.begin(), tExpectedObjective.end(), aStartingValue);

    auto tGradientAsVector = std::vector<double>(aNumberOfDesignVariables.mValue);
    std::copy(aGradientView.begin(), aGradientView.end(), tGradientAsVector.begin());

    EXPECT_EQ(tGradientAsVector, tExpectedObjective) << aTestContext;
}

}  // namespace

TEST(ObjectiveConstraintGradientArrayView, Sizes)
{
    constexpr auto tNumberOfConstraints = ConstraintSizeType{std::size_t{2}};
    constexpr auto tNumberOfDesignVariables = DesignVariableSizeType{std::size_t{3}};
    const auto tTestData = test_gradient_array(tNumberOfConstraints, tNumberOfDesignVariables);
    const auto tGradientView =
        ObjectiveConstraintGradientArrayView{tTestData.data(), tNumberOfConstraints, tNumberOfDesignVariables};

    const auto tExpectedSize = (tNumberOfConstraints.mValue + 1) * tNumberOfDesignVariables.mValue;
    EXPECT_EQ(tGradientView.arraySize(), tExpectedSize);
    EXPECT_EQ(tGradientView.numberOfConstraints().mValue, tNumberOfConstraints.mValue);
    EXPECT_EQ(tGradientView.numberOfDesignVariables().mValue, tNumberOfDesignVariables.mValue);
}

TEST(ObjectiveConstraintGradientArrayView, ConstObjective)
{
    constexpr auto tNumberOfConstraints = ConstraintSizeType{std::size_t{2}};
    constexpr auto tNumberOfDesignVariables = DesignVariableSizeType{std::size_t{3}};
    const auto tTestData = test_gradient_array(tNumberOfConstraints, tNumberOfDesignVariables);
    const auto tGradientView =
        ObjectiveConstraintGradientArrayView{tTestData.data(), tNumberOfConstraints, tNumberOfDesignVariables};

    constexpr auto tStartingValue = 0.0;
    check_gradient(tGradientView.objectiveGradient(), tNumberOfDesignVariables, tStartingValue,
                   TEST_CONTEXT("Const objective"));
}

TEST(ObjectiveConstraintGradientArrayView, Objective)
{
    constexpr auto tNumberOfConstraints = ConstraintSizeType{std::size_t{1}};
    constexpr auto tNumberOfDesignVariables = DesignVariableSizeType{std::size_t{2}};

    auto tTestData = std::vector<double>(array_size(tNumberOfConstraints, tNumberOfDesignVariables));
    auto tGradientView =
        ObjectiveConstraintGradientArrayView{tTestData.data(), tNumberOfConstraints, tNumberOfDesignVariables};
    std::iota(tGradientView.objectiveGradient().begin(), tGradientView.objectiveGradient().end(), 0.0);

    constexpr auto tStartingValue = 0.0;
    check_gradient(tGradientView.objectiveGradient(), tNumberOfDesignVariables, tStartingValue,
                   TEST_CONTEXT("Non-const objective"));
}

TEST(ObjectiveConstraintGradientArrayView, ConstConstraints)
{
    constexpr auto tNumberOfConstraints = ConstraintSizeType{std::size_t{2}};
    constexpr auto tNumberOfDesignVariables = DesignVariableSizeType{std::size_t{3}};
    const auto tTestData = test_gradient_array(tNumberOfConstraints, tNumberOfDesignVariables);
    const auto tGradientView =
        ObjectiveConstraintGradientArrayView{tTestData.data(), tNumberOfConstraints, tNumberOfDesignVariables};

    constexpr auto tStartingValue0 = static_cast<double>(tNumberOfDesignVariables.mValue);
    check_gradient(tGradientView.constraintGradient(ConstraintSizeType{0}), tNumberOfDesignVariables, tStartingValue0,
                   TEST_CONTEXT("Const constraint"));

    constexpr auto tStartingValue1 = static_cast<double>(2 * tNumberOfDesignVariables.mValue);
    check_gradient(tGradientView.constraintGradient(ConstraintSizeType{1}), tNumberOfDesignVariables, tStartingValue1,
                   TEST_CONTEXT("Const constraint"));
}

TEST(ObjectiveConstraintGradientArrayView, Constraint)
{
    constexpr auto tNumberOfConstraints = ConstraintSizeType{std::size_t{1}};
    constexpr auto tNumberOfDesignVariables = DesignVariableSizeType{std::size_t{2}};

    auto tTestData = std::vector<double>(array_size(tNumberOfConstraints, tNumberOfDesignVariables));
    auto tGradientView =
        ObjectiveConstraintGradientArrayView{tTestData.data(), tNumberOfConstraints, tNumberOfDesignVariables};
    auto tConstraintView = tGradientView.constraintGradient(ConstraintSizeType{0});
    std::iota(tConstraintView.begin(), tConstraintView.end(), 0.0);

    constexpr auto tStartingValue = 0.0;
    check_gradient(tConstraintView, tNumberOfDesignVariables, tStartingValue, TEST_CONTEXT("Non-const constraint"));
}

}  // namespace plato::third_party_integration::snopt::unittest
