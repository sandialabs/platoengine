#ifndef PLATO_ROLINTEGRATION_UNITTEST_HIMMELBLAUHELPERS
#define PLATO_ROLINTEGRATION_UNITTEST_HIMMELBLAUHELPERS

#include <gtest/gtest.h>

#include "plato/output/OutputManager.hpp"
#include "plato/test_utilities/Himmelblau.hpp"
#include "plato/third_party_integration/rol/ROLObjectiveFunction.hpp"

namespace plato::third_party_integration::rol::unittest
{
namespace detail
{

constexpr auto kControlX = 1.2;
constexpr auto kControlY = 0.5;

constexpr auto kGoldValue = test_utilities::Himmelblau{}.f(kControlX, kControlY);
constexpr auto kGoldGradientX = test_utilities::Himmelblau{}.df(kControlX, kControlY).mData.at(0);
constexpr auto kGoldGradientY = test_utilities::Himmelblau{}.df(kControlX, kControlY).mData.at(1);

}  // namespace detail

template <typename Argument>
void run_himmelblau_objective_test(const Argument& aObjectiveFunctionArgument)
{
    using namespace detail;
    auto tObjective = ROLObjectiveFunction{aObjectiveFunctionArgument};

    const auto tControl = std::vector<double>{kControlX, kControlY};
    double tTolerance{};
    EXPECT_DOUBLE_EQ(tObjective.value(tControl, tTolerance), kGoldValue);

    auto tGradient = std::vector<double>{0.0, 0.0};
    tObjective.gradient(tGradient, tControl, tTolerance);
    EXPECT_DOUBLE_EQ(tGradient[0], kGoldGradientX);
    EXPECT_DOUBLE_EQ(tGradient[1], kGoldGradientY);

    constexpr auto tGradientSize = 2U;
    auto tHessian = std::vector<double>(tGradientSize);
    auto tDirection = std::vector<double>{1.0, 1.0};
    tObjective.hessVec(tHessian, tDirection, tControl, tTolerance);
    EXPECT_EQ(tHessian[0], 0.0);
    EXPECT_EQ(tHessian[1], 0.0);
}

template <typename Argument>
void run_himmelblau_output_test(const Argument& aObjectiveFunctionArgument)
{
    using namespace detail;

    std::size_t tOutputCount = 0U;
    auto tOutputFunction = [&tOutputCount](const linear_algebra::DynamicVector<double>&, const output::OutputInfo&)
    { tOutputCount++; };
    auto tOutputManager = output::OutputManager{tOutputFunction, output::OutputMode::kEveryIterationAppend};
    auto tObjective = ROLObjectiveFunction{aObjectiveFunctionArgument, tOutputManager};

    const auto tControl = std::vector<double>{kControlX, kControlY};
    constexpr int tBogusIteration = 1;

    tObjective.update(tControl, ROL::UpdateType::Initial, tBogusIteration);
    EXPECT_EQ(tOutputCount, 1U);  // UpdateType::Initial triggers call to output function
    tObjective.update(tControl, ROL::UpdateType::Accept, tBogusIteration);
    EXPECT_EQ(tOutputCount, 2U);  // UpdateType::Accept triggers call to output function
    tObjective.update(tControl, ROL::UpdateType::Revert,
                      tBogusIteration);  // UpdateType::Revert doesn't trigger call to output function
    EXPECT_EQ(tOutputCount, 2U);
    tObjective.update(tControl, ROL::UpdateType::Trial,
                      tBogusIteration);  // UpdateType::TrialRevert doesn't trigger call to output function
    EXPECT_EQ(tOutputCount, 2U);
    tObjective.update(tControl, ROL::UpdateType::Temp,
                      tBogusIteration);  // UpdateType::Temp doesn't trigger call to output function
    EXPECT_EQ(tOutputCount, 2U);

    tObjective.finalUpdate(linear_algebra::DynamicVector(tControl));  // increments the count
    EXPECT_EQ(tOutputCount, 3U);
}

}  // namespace plato::third_party_integration::rol::unittest

#endif
