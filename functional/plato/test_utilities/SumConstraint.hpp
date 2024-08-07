#ifndef PLATO_TESTUTILITIES_SUMCONSTRAINT
#define PLATO_TESTUTILITIES_SUMCONSTRAINT

#include <cmath>

#include "plato/test_utilities/TwoDTestTypes.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::test_utilities
{
class SumConstraint
{
   public:
    constexpr SumConstraint() = default;
    constexpr SumConstraint(const std::pair<double, double> aCenter, const unsigned int aPower)
        : mCenter(aCenter), mPower(aPower)
    {
        if (mPower == 0)
        {
            throw utilities::Exception("Power must be 1 or greater");
        }
    }

    [[nodiscard]] constexpr double f(const double aX, const double aY) const
    {
        const double tXTerm = aX - mCenter.first;
        const double tYTerm = aY - mCenter.second;
        return std::pow(tXTerm, mPower) + std::pow(tYTerm, mPower);
    }

    [[nodiscard]] constexpr TwoDVector df(const double aX, const double aY) const
    {
        const double tXTerm = (aX - mCenter.first);
        const double tYTerm = (aY - mCenter.second);

        return makeTwoDVector(mPower * std::pow(tXTerm, mPower - 1), mPower * std::pow(tYTerm, mPower - 1));
    }

   private:
    std::pair<double, double> mCenter{0, 0};
    unsigned int mPower = 1u;
};

}  // namespace plato::test_utilities

#endif
