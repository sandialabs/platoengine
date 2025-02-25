#ifndef PLATO_TEST_UTILITIES_HIMMELBLAU
#define PLATO_TEST_UTILITIES_HIMMELBLAU

#include <type_traits>

#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::test_utilities
{

struct Himmelblau
{
    double mA = 11.0;
    double mB = 7.0;
    [[nodiscard]] constexpr double f(const double aX, const double aY) const
    {
        const double tParensOne = aX * aX + aY - mA;
        const double tParensTwo = aX + aY * aY - mB;
        return tParensOne * tParensOne + tParensTwo * tParensTwo;
    }

    [[nodiscard]] constexpr TwoDVector df(const double aX, const double aY) const
    {
        const double tParensOne = aX * aX + aY - mA;
        const double tParensTwo = aX + aY * aY - mB;

        const double tXterm = 4 * aX * tParensOne + 2 * tParensTwo;
        const double tYterm = 2 * tParensOne + 4 * aY * tParensTwo;

        return makeTwoDVector(tXterm, tYterm);
    }
};

}  // namespace plato::test_utilities

#endif
