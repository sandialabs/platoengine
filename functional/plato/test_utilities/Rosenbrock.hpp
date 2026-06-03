#ifndef PLATO_TEST_UTILITIES_ROSENBROCK
#define PLATO_TEST_UTILITIES_ROSENBROCK

#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::test_utilities
{
/// @brief Implementation of the 2D Rosenbrock function
///
/// This is mainly useful for testing optimization problems.
struct Rosenbrock
{
    double mA = 1.0;
    double mB = 100.0;

    [[nodiscard]] constexpr auto f(const double aXComponent, const double aYComponent) const -> double
    {
        const double tAMinusX = mA - aXComponent;
        const double tYMinusX2 = aYComponent - aXComponent * aXComponent;
        return tAMinusX * tAMinusX + mB * tYMinusX2 * tYMinusX2;
    }

    [[nodiscard]] constexpr auto df(const double aXComponent, const double aYComponent) const -> TwoDVector
    {
        constexpr auto tSquarePower = 2.0;
        constexpr auto tQuarticPower = 4.0;

        return make_two_d_vector(-tSquarePower * (mA - aXComponent) -
                                     tQuarticPower * mB * aXComponent * (aYComponent - aXComponent * aXComponent),
                                 tSquarePower * mB * (aYComponent - aXComponent * aXComponent));
    }
};

}  // namespace plato::test_utilities

#endif
