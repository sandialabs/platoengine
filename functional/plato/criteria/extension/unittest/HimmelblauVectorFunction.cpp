#include "plato/criteria/extension/unittest/HimmelblauVectorFunction.hpp"

#include <utility>

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/test_utilities/Himmelblau.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::criteria::extension::unittest
{

namespace
{

struct TwoHimmelblauFunctionBase
{
    test_utilities::Himmelblau mFirstHimmelblau;
    test_utilities::Himmelblau mSecondHimmelblau;
    [[nodiscard]] auto evaluateHimmelblauDerivatives(const linear_algebra::DynamicVector<double>& aX) const
        -> std::pair<test_utilities::TwoDVector, test_utilities::TwoDVector>
    {
        const auto& tStdVectorX = aX.stdVector();
        const auto tDHimmelblauOne = mFirstHimmelblau.df(tStdVectorX.front(), tStdVectorX.back());
        const auto tDHimmelblauTwo = mSecondHimmelblau.df(tStdVectorX.front(), tStdVectorX.back());
        return std::make_pair(tDHimmelblauOne, tDHimmelblauTwo);
    }
};

struct TwoHimmelblauFunctions : public TwoHimmelblauFunctionBase
{
    [[nodiscard]] auto operator()(const linear_algebra::DynamicVector<double>& aX) const
        -> linear_algebra::DynamicVector<double>
    {
        return linear_algebra::DynamicVector<double>{
            {mFirstHimmelblau.f(aX.stdVector().front(), aX.stdVector().back()),
             mSecondHimmelblau.f(aX.stdVector().front(), aX.stdVector().back())}};
    }
};

struct TwoHimmelblauFunctionsJacobian : public TwoHimmelblauFunctionBase
{
    [[nodiscard]] auto operator()(const linear_algebra::DynamicVector<double>& aX) const
        -> linear_algebra::JacobianMultiplier
    {
        const auto tHimmelblauDerivatives = evaluateHimmelblauDerivatives(aX);
        return linear_algebra::JacobianMultiplier{
            [tDHimmelblauOne = tHimmelblauDerivatives.first,
             tDHimmelblauTwo = tHimmelblauDerivatives.second](const linear_algebra::DynamicVector<double>& aRowVector)
            {
                const auto tFirstEntry = aRowVector.stdVector().front() * tDHimmelblauOne.mData[0] +
                                         aRowVector.stdVector().back() * tDHimmelblauTwo.mData[0];
                const auto tSecondEntry = aRowVector.stdVector().front() * tDHimmelblauOne.mData[1] +
                                          aRowVector.stdVector().back() * tDHimmelblauTwo.mData[1];
                return linear_algebra::DynamicVector<double>{{tFirstEntry, tSecondEntry}};
            }};
    }
};

[[nodiscard]] auto adjoint_jacobian_entry(const linear_algebra::DynamicVector<double>& aRowVector,
                                          const test_utilities::TwoDVector& aDf) -> double
{
    return aRowVector.stdVector().front() * aDf.mData[0] + aRowVector.stdVector().back() * aDf.mData[1];
}

struct TwoHimmelblauFunctionsAdjointJacobian : public TwoHimmelblauFunctionBase
{
    [[nodiscard]] auto operator()(const linear_algebra::DynamicVector<double>& aX) const
        -> linear_algebra::AdjointJacobianMultiplier
    {
        const auto tHimmelblauDerivatives = evaluateHimmelblauDerivatives(aX);
        return linear_algebra::AdjointJacobianMultiplier{linear_algebra::JacobianMultiplier{
            [tDHimmelblauOne = tHimmelblauDerivatives.first,
             tDHimmelblauTwo = tHimmelblauDerivatives.second](const linear_algebra::DynamicVector<double>& aRowVector)
            {
                const auto tFirstEntry = adjoint_jacobian_entry(aRowVector, tDHimmelblauOne);
                const auto tSecondEntry = adjoint_jacobian_entry(aRowVector, tDHimmelblauTwo);
                return linear_algebra::DynamicVector<double>{{tFirstEntry, tSecondEntry}};
            }}};
    }
};
}  // namespace

auto make_vector_himmelblau_dynamic_vector_objective_function(const FirstHimmelblau& aFirstHimmelblau,
                                                              const SecondHimmelblau& aSecondHimmelblau)
    -> VectorObjectiveFunction
{
    return VectorObjectiveFunction{
        TwoHimmelblauFunctions{aFirstHimmelblau.mValue, aSecondHimmelblau.mValue},
        TwoHimmelblauFunctionsJacobian{aFirstHimmelblau.mValue, aSecondHimmelblau.mValue},
        TwoHimmelblauFunctionsAdjointJacobian{aFirstHimmelblau.mValue, aSecondHimmelblau.mValue}};
}

}  // namespace plato::criteria::extension::unittest
