#ifndef PLATO_TEST_UTILITIES_ROSENBROCK
#define PLATO_TEST_UTILITIES_ROSENBROCK

#include <type_traits>

#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::test_utilities
{
class Rosenbrock
{
   public:
    Rosenbrock() = default;
    Rosenbrock(const double aA, const double aB) : mA(aA), mB(aB) {}

    [[nodiscard]] double f(const double aX, const double aY) const
    {
        const double aMinusX = mA - aX;
        const double yMinusX2 = aY - aX * aX;
        return aMinusX * aMinusX + mB * yMinusX2 * yMinusX2;
    }

    [[nodiscard]] TwoDVector df(const double aX, const double aY) const
    {
        return makeTwoDVector(-2.0 * (mA - aX) - 4.0 * mB * aX * (aY - aX * aX), 2.0 * mB * (aY - aX * aX));
    }

   private:
    double mA = 1.0;
    double mB = 100.0;
};

}  // namespace plato::test_utilities

#endif
