#ifndef PLATO_LINEAR_ALGEBRA_JACOBIANMULTIPLIER
#define PLATO_LINEAR_ALGEBRA_JACOBIANMULTIPLIER

#include <iostream>

#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::linear_algebra
{
namespace
{
void print(const std::vector<double>& aVector, const std::string& aName)
{
    std::cout << aName << std::endl;
    std::cout << "Avector size : " << aVector.size() << std::endl;
    for (const auto& x : aVector)
    {
        std::cout << x << " ";
    }
    std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    std::cout << std::endl;
}
}  // namespace

/// @brief An object representing the multiplication of a row vector and a Jacobian matrix.
struct JacobianMultiplier
{
    using JacobianTimesVectorFunction = std::function<DynamicVector<double>(const DynamicVector<double>&)>;

    JacobianTimesVectorFunction mJacobianTimesVectorFunction;

    /// @brief Implementation of multiplication of two JacobianMultipliers
    JacobianMultiplier operator*(const JacobianMultiplier& aG) const
    {
        auto tComposedFunction = [tF = *this, tG = aG](const DynamicVector<double>& aDirection)
        {
            print(aDirection.stdVector(), "aDirection: ");
            print(tG.mJacobianTimesVectorFunction(aDirection).stdVector(), "tG.mJacobianTimesvectorFunction: ");
            print(tF.mJacobianTimesVectorFunction(tG.mJacobianTimesVectorFunction(aDirection)).stdVector(),
                  "tF.mJacobianTimesVectorFunction(tG.mJacobianTimesVectorFunction(aDirection)): ");
            return tF.mJacobianTimesVectorFunction(tG.mJacobianTimesVectorFunction(aDirection));
        };

        return JacobianMultiplier{tComposedFunction};
    }
};

/// @brief Implementation of multiplication of a row vector @a aX
template <typename Arg>
[[nodiscard]] DynamicVector<double> operator*(const Arg& aX, const JacobianMultiplier& aA)
{
    return aA.mJacobianTimesVectorFunction(aX);
}

}  // namespace plato::linear_algebra

#endif
