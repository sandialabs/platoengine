#ifndef PLATO_TEST_UTILITIES_HIMMELBLAU
#define PLATO_TEST_UTILITIES_HIMMELBLAU

#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::test_utilities
{

struct Himmelblau
{
    constexpr static double mDefaultAParameter = 11.0;
    constexpr static double mDefaultBParameter = 7.0;

    double mAParameter = mDefaultAParameter;
    double mBParameter = mDefaultBParameter;

    [[nodiscard]] constexpr auto f(const double aXArg, const double aYArg) const -> double
    {
        const double tParensOne = aXArg * aXArg + aYArg - mAParameter;
        const double tParensTwo = aXArg + aYArg * aYArg - mBParameter;
        return tParensOne * tParensOne + tParensTwo * tParensTwo;
    }

    [[nodiscard]] constexpr auto df(const double aXArg, const double aYArg) const -> TwoDVector
    {
        const double tParensOne = aXArg * aXArg + aYArg - mAParameter;
        const double tParensTwo = aXArg + aYArg * aYArg - mBParameter;

        const double tXterm = 4 * aXArg * tParensOne + 2 * tParensTwo;
        const double tYterm = 2 * tParensOne + 4 * aYArg * tParensTwo;

        return make_two_d_vector(tXterm, tYterm);
    }
};

}  // namespace plato::test_utilities

#endif
