#ifndef PLATO_TEST_UTILITIES_PENALTY
#define PLATO_TEST_UTILITIES_PENALTY

#include <cmath>

#include "plato/test_utilities/TwoDTestTypes.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::test_utilities
{
using XMin = utilities::NamedType<double, struct XMinTag>;
using Exponent = utilities::NamedType<double, struct ExponentTag>;

class Penalty
{
   public:
    Penalty(const XMin aXMin, const Exponent aPower) : mXMin(aXMin.mValue), mPower(aPower.mValue) {}

    [[nodiscard]] auto f(const double aX, const double aY) const -> TwoDVector
    {
        return makeTwoDVector(mXMin + (1.0 - mXMin) * std::pow(aX, mPower),
                              mXMin + (1.0 - mXMin) * std::pow(aY, mPower));
    }

    [[nodiscard]] auto df(const double aX, const double aY) const -> TwoDMatrix
    {
        return makeTwoDMatrix(mPower * (1.0 - mXMin) * std::pow(aX, mPower - 1.0), 0.0, 0.0,
                              mPower * (1.0 - mXMin) * std::pow(aY, mPower - 1.0));
    }

   private:
    double mXMin{};
    double mPower{};
};

}  // namespace plato::test_utilities

#endif
