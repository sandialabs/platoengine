#ifndef PLATO_ROLINTEGRATION_UNITTEST_HIMMELBLAUHELPERS
#define PLATO_ROLINTEGRATION_UNITTEST_HIMMELBLAUHELPERS

#include <gtest/gtest.h>

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

    const auto tControl = ROL::StdVector<double>{kControlX, kControlY};
    double tTolerance;
    EXPECT_DOUBLE_EQ(tObjective.value(tControl, tTolerance), kGoldValue);

    auto tGradient = ROL::StdVector<double>{0.0, 0.0};
    tObjective.gradient(tGradient, tControl, tTolerance);
    EXPECT_DOUBLE_EQ(tGradient[0], kGoldGradientX);
    EXPECT_DOUBLE_EQ(tGradient[1], kGoldGradientY);
}

}  // namespace plato::third_party_integration::rol::unittest

#endif
