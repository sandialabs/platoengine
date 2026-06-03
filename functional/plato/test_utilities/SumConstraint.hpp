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
    constexpr SumConstraint(std::pair<double, double> aCenter, unsigned int aPower);

    [[nodiscard]] constexpr auto f(double aXComponent, double aYComponent) const -> double;
    [[nodiscard]] constexpr auto df(double aXComponent, double aYComponent) const -> TwoDVector;

   private:
    std::pair<double, double> mCenter{0, 0};
    unsigned int mPower = 1U;
};

constexpr SumConstraint::SumConstraint(const std::pair<double, double> aCenter, const unsigned int aPower)
    : mCenter(aCenter), mPower(aPower)
{
    assert(mPower != 0);
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
constexpr auto SumConstraint::f(const double aXComponent, const double aYComponent) const -> double
{
    const double tXTerm = aXComponent - mCenter.first;
    const double tYTerm = aYComponent - mCenter.second;
    return std::pow(tXTerm, mPower) + std::pow(tYTerm, mPower);
}

constexpr auto SumConstraint::df(const double aXComponent, const double aYComponent) const -> TwoDVector
{
    const double tXTerm = (aXComponent - mCenter.first);
    const double tYTerm = (aYComponent - mCenter.second);

    return make_two_d_vector(mPower * std::pow(tXTerm, mPower - 1), mPower * std::pow(tYTerm, mPower - 1));
}
// NOLINTEND(bugprone-easily-swappable-parameters)

}  // namespace plato::test_utilities

#endif
