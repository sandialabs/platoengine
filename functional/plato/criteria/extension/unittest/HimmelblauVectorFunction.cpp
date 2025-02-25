#include "plato/criteria/extension/unittest/HimmelblauVectorFunction.hpp"

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/Himmelblau.hpp"

namespace plato::criteria::extension::unittest
{

namespace
{

struct TwoHimmelblauFunctionBase
{
    test_utilities::Himmelblau mFirstHimmelblau;
    test_utilities::Himmelblau mSecondHimmelblau;
    [[nodiscard]] auto evaluateHimmelblauDerivatives(const linear_algebra::DynamicVector<double>& x) const
        -> std::pair<test_utilities::TwoDVector, test_utilities::TwoDVector>
    {
        const auto tStdVectorX = x.stdVector();
        const auto tDHimmelblauOne = mFirstHimmelblau.df(tStdVectorX.front(), tStdVectorX.back());
        const auto tDHimmelblauTwo = mSecondHimmelblau.df(tStdVectorX.front(), tStdVectorX.back());
        return std::make_pair(tDHimmelblauOne, tDHimmelblauTwo);
    }
};

struct TwoHimmelblauFunctions : public TwoHimmelblauFunctionBase
{
    [[nodiscard]] linear_algebra::DynamicVector<double> operator()(const linear_algebra::DynamicVector<double>& x) const
    {
        return linear_algebra::DynamicVector<double>{
            {mFirstHimmelblau.f(x.stdVector().front(), x.stdVector().back()),
             mSecondHimmelblau.f(x.stdVector().front(), x.stdVector().back())}};
    }
};

struct TwoHimmelblauFunctionsJacobian : public TwoHimmelblauFunctionBase
{
    [[nodiscard]] linear_algebra::JacobianMultiplier operator()(const linear_algebra::DynamicVector<double>& x) const
    {
        const auto tHimmelblauDerivatives = evaluateHimmelblauDerivatives(x);
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

[[nodiscard]] double adjoint_jacobian_entry(const linear_algebra::DynamicVector<double>& aRowVector,
                                            const test_utilities::TwoDVector& aDf)
{
    return aRowVector.stdVector().front() * aDf.mData[0] + aRowVector.stdVector().back() * aDf.mData[1];
}

struct TwoHimmelblauFunctionsAdjointJacobian : public TwoHimmelblauFunctionBase
{
    [[nodiscard]] linear_algebra::AdjointJacobianMultiplier operator()(
        const linear_algebra::DynamicVector<double>& x) const
    {
        const auto tHimmelblauDerivatives = evaluateHimmelblauDerivatives(x);
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
