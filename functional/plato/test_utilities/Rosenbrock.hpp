#ifndef PLATO_TEST_UTILITIES_ROSENBROCK
#define PLATO_TEST_UTILITIES_ROSENBROCK

#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::test_utilities
{
/// @brief Implementation of the 2D Rosenbrock function
///
/// This is mainly useful for testing optimization problems.
class Rosenbrock
{
   public:
    constexpr Rosenbrock() noexcept = default;
    constexpr Rosenbrock(const double aA, const double aB) noexcept : mA(aA), mB(aB) {}

    [[nodiscard]] constexpr auto f(const double aX, const double aY) const -> double
    {
        const double aMinusX = mA - aX;
        const double yMinusX2 = aY - aX * aX;
        return aMinusX * aMinusX + mB * yMinusX2 * yMinusX2;
    }

    [[nodiscard]] constexpr auto df(const double aX, const double aY) const -> TwoDVector
    {
        return makeTwoDVector(-2.0 * (mA - aX) - 4.0 * mB * aX * (aY - aX * aX), 2.0 * mB * (aY - aX * aX));
    }

   private:
    double mA = 1.0;
    double mB = 100.0;
};

}  // namespace plato::test_utilities

#endif
