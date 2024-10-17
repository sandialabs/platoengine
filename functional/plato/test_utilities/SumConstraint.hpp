#ifndef PLATO_TESTUTILITIES_SUMCONSTRAINT
#define PLATO_TESTUTILITIES_SUMCONSTRAINT

#include <cassert>
#include <cmath>

#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::test_utilities
{
/// @brief A class for testing constraints, this implements the function \f$f = (x - a)^p + (y - b)^p\f$ and its
/// gradient.
class SumConstraint
{
   public:
    /// @brief Initializes exponent \f$p\f$ to 1 and centers \f$a\f$ and \f$b\f$ to 0.
    constexpr SumConstraint() = default;
    /// @brief Initializes exponent \f$p\f$ to @a aPower and centers \f$a\f$ and \f$b\f$ to `first` and `second` of @a
    /// aCenter respectively.
    /// @pre @a aPower must not be 0.
    constexpr SumConstraint(const std::pair<double, double> aCenter, const unsigned int aPower);

    [[nodiscard]] constexpr auto f(const double aX, const double aY) const -> double;
    [[nodiscard]] constexpr auto df(const double aX, const double aY) const -> TwoDVector;

   private:
    std::pair<double, double> mCenter{0, 0};
    unsigned int mPower = 1u;
};

constexpr SumConstraint::SumConstraint(const std::pair<double, double> aCenter, const unsigned int aPower)
    : mCenter(aCenter), mPower(aPower)
{
    assert(mPower != 0);
}

constexpr double SumConstraint::f(const double aX, const double aY) const
{
    const double tXTerm = aX - mCenter.first;
    const double tYTerm = aY - mCenter.second;
    return std::pow(tXTerm, mPower) + std::pow(tYTerm, mPower);
}

constexpr TwoDVector SumConstraint::df(const double aX, const double aY) const
{
    const double tXTerm = (aX - mCenter.first);
    const double tYTerm = (aY - mCenter.second);

    return makeTwoDVector(mPower * std::pow(tXTerm, mPower - 1), mPower * std::pow(tYTerm, mPower - 1));
}

}  // namespace plato::test_utilities

#endif
