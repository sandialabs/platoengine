#ifndef PLATO_ROLINTEGRATION_UNITTEST_HIMMELBLAUHELPERS
#define PLATO_ROLINTEGRATION_UNITTEST_HIMMELBLAUHELPERS

#include <gtest/gtest.h>

#include "plato/output/OutputManager.hpp"
#include "plato/third_party_integration/rol/ROLObjectiveFunction.hpp"

namespace plato::third_party_integration::rol::unittest
{
namespace detail
{

constexpr double kControlX = 1.2;
constexpr double kControlY = .5;
[[nodiscard]] constexpr double himmelblau_term_one(double aX, double aY) { return aX * aX + aY - 11.0; }
[[nodiscard]] constexpr double himmelblau_term_two(double aX, double aY) { return aX + aY * aY - 7.0; }
[[nodiscard]] constexpr double himmelblau_value(double aX, double aY)
{
    return himmelblau_term_one(aX, aY) * himmelblau_term_one(aX, aY) +
           himmelblau_term_two(aX, aY) * himmelblau_term_two(aX, aY);
}
[[nodiscard]] constexpr double himmelblau_derivative_x(double aX, double aY)
{
    return 4 * aX * himmelblau_term_one(aX, aY) + 2 * himmelblau_term_two(aX, aY);
}
[[nodiscard]] constexpr double himmelblau_derivative_y(double aX, double aY)
{
    return 2 * himmelblau_term_one(aX, aY) + 4 * aY * himmelblau_term_two(aX, aY);
}
constexpr double kGoldValue = himmelblau_value(kControlX, kControlY);
constexpr double kGoldGradientX = himmelblau_derivative_x(kControlX, kControlY);
constexpr double kGoldGradientY = himmelblau_derivative_y(kControlX, kControlY);

}  // namespace detail

template <typename Argument>
void run_himmelblau_objective_test(const Argument& aObjectiveFunctionArgument)
{
    using namespace detail;
    auto tObjective = ROLObjectiveFunction{aObjectiveFunctionArgument};

    const auto tControl = std::vector<double>{kControlX, kControlY};
    double tTolerance;
    EXPECT_DOUBLE_EQ(tObjective.value(tControl, tTolerance), kGoldValue);

    auto tGradient = std::vector<double>{0.0, 0.0};
    tObjective.gradient(tGradient, tControl, tTolerance);
    EXPECT_DOUBLE_EQ(tGradient[0], kGoldGradientX);
    EXPECT_DOUBLE_EQ(tGradient[1], kGoldGradientY);

    auto tHessian = std::vector<double>{10.0, 10.0};
    auto tDirection = std::vector<double>{1.0, 1.0};
    tObjective.hessVec(tHessian, tDirection, tControl, tTolerance);
    EXPECT_EQ(tHessian[0], 0.0);
    EXPECT_EQ(tHessian[1], 0.0);
}

template <typename Argument>
void run_himmelblau_output_test(const Argument& aObjectiveFunctionArgument)
{
    using namespace detail;

    std::size_t tOutputCount = 0u;
    auto tOutputFunction = [&tOutputCount](const linear_algebra::DynamicVector<double>&, const output::OutputInfo&)
    { tOutputCount++; };
    auto tOutputManager = output::OutputManager{tOutputFunction, output::OutputMode::kEveryIterationAppend};
    auto tObjective = ROLObjectiveFunction{aObjectiveFunctionArgument, tOutputManager};

    const auto tControl = std::vector<double>{kControlX, kControlY};
    constexpr int tBogusIteration = 1;

    tObjective.update(tControl, ROL::UpdateType::Initial, tBogusIteration);
    EXPECT_EQ(tOutputCount, 1u);  // UpdateType::Initial triggers call to output function
    tObjective.update(tControl, ROL::UpdateType::Accept, tBogusIteration);
    EXPECT_EQ(tOutputCount, 2u);  // UpdateType::Accept triggers call to output function
    tObjective.update(tControl, ROL::UpdateType::Revert,
                      tBogusIteration);  // UpdateType::Revert doesn't trigger call to output function
    EXPECT_EQ(tOutputCount, 2u);
    tObjective.update(tControl, ROL::UpdateType::Trial,
                      tBogusIteration);  // UpdateType::TrialRevert doesn't trigger call to output function
    EXPECT_EQ(tOutputCount, 2u);
    tObjective.update(tControl, ROL::UpdateType::Temp,
                      tBogusIteration);  // UpdateType::Temp doesn't trigger call to output function
    EXPECT_EQ(tOutputCount, 2u);

    tObjective.finalUpdate(linear_algebra::DynamicVector(tControl));  // increments the count
    EXPECT_EQ(tOutputCount, 3u);
}

}  // namespace plato::third_party_integration::rol::unittest

#endif
