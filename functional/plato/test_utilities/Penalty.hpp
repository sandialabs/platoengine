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
    constexpr Penalty(const XMin aXMin, const Exponent aPower) : mXMin(aXMin.mValue), mPower(aPower.mValue) {}

    [[nodiscard]] auto f(const double aXCoordinate, const double aYCoordinate) const -> TwoDVector
    {
        return make_two_d_vector(mXMin + (1.0 - mXMin) * std::pow(aXCoordinate, mPower),
                                 mXMin + (1.0 - mXMin) * std::pow(aYCoordinate, mPower));
    }

    [[nodiscard]] auto df(const double aXCoordinate, const double aYCoordinate) const -> TwoDMatrix
    {
        return make_two_d_matrix(mPower * (1.0 - mXMin) * std::pow(aXCoordinate, mPower - 1.0), 0.0, 0.0,
                                 mPower * (1.0 - mXMin) * std::pow(aYCoordinate, mPower - 1.0));
    }

   private:
    double mXMin{};
    double mPower{};
};

}  // namespace plato::test_utilities

#endif
