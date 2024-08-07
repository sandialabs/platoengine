#include <gtest/gtest.h>

#include <cmath>

#include "plato/core/Compose.hpp"
#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::linear_algebra::unittest
{
using VectorFunction = core::Function<DynamicVector<double>, JacobianMultiplier, const DynamicVector<double>&>;

namespace
{
const auto kFirstFunction = [](const double& aX, const double& aY) { return aX * aX + aY * aX; };

const auto kFirstFunctionGrad = [](const DynamicVector<double>& aV)
{
    const double tX = aV[0];
    const double tY = aV[1];
    return DynamicVector{{2.0 * tX + tY, tX}};
};

const auto kSecondFunction = [](const double& aX, const double& aY) { return aY * aY + aX; };

const auto kSecondFunctionGrad = [](const DynamicVector<double>& aV)
{
    const double tY = aV[1];
    return DynamicVector({1.0, 2.0 * tY});
};

const auto kThirdFunction = [](const double& aX, const double& aY) { return aY * aY + aX * aX; };

const auto kThirdFunctionGrad = [](const DynamicVector<double>& aV)
{
    const double tX = aV[0];
    const double tY = aV[1];
    return DynamicVector({2.0 * tX, 2.0 * tY});
};

const auto kForthFunction = [](const double& aX, const double& aY, const double& aZ)
{ return aY * aY + aX * aX + aZ * aZ; };
const auto kForthFunctionGrad = [](const DynamicVector<double>& aV)
{
    const double tX = aV[0];
    const double tY = aV[1];
    const double tZ = aV[1];
    return DynamicVector({2.0 * tX, 2.0 * tY, 2.0 * tZ});
};

const auto kFifthFunction = [](const double& aX, const double& aY, const double& aZ)
{ return aY * aZ + aX * aY + aZ * aX; };
const auto kFifthFunctionGrad = [](const DynamicVector<double>& aV)
{
    const double tX = aV[0];
    const double tY = aV[1];
    const double tZ = aV[1];
    return DynamicVector({tY + tZ, tZ + tX, tY + tX});
};

auto make_jacobian_multiplier_two_to_three(const DynamicVector<double>& aX) -> JacobianMultiplier
{
    const unsigned int tNumberOfColumns = 3;
    const auto tFirstFunctionGrad = kFirstFunctionGrad(aX);
    const auto tSecondFunctionGrad = kSecondFunctionGrad(aX);
    const auto tThirdFunctionGrad = kThirdFunctionGrad(aX);

    const JacobianMultiplier::JacobianTimesVectorFunction tJacobianTimesVectorFunction =
        [tFirstFunctionGrad, tSecondFunctionGrad, tThirdFunctionGrad](const DynamicVector<double>& aVector)
    {
        return DynamicVector<double>(
            {tFirstFunctionGrad.dot(aVector), tSecondFunctionGrad.dot(aVector), tThirdFunctionGrad.dot(aVector)});
    };

    return {tNumberOfColumns, tJacobianTimesVectorFunction};
}

auto make_jacobian_multiplier_three_to_two(const DynamicVector<double>& aX) -> JacobianMultiplier
{
    const unsigned int tNumberOfColumns = 2;
    const auto tForthFunctionGrad = kForthFunctionGrad(aX);
    const auto tFifthFunctionGrad = kFifthFunctionGrad(aX);

    const JacobianMultiplier::JacobianTimesVectorFunction tJacobianTimesVectorFunction =
        [tForthFunctionGrad, tFifthFunctionGrad](const DynamicVector<double>& aVector) {
            return DynamicVector<double>({tForthFunctionGrad.dot(aVector), tFifthFunctionGrad.dot(aVector)});
        };

    return {tNumberOfColumns, tJacobianTimesVectorFunction};
}

auto create_vector_function_two_to_three() -> VectorFunction
{
    return core::make_function(
        [](const DynamicVector<double>& aX)
        {
            const double tX = aX[0];
            const double tY = aX[1];
            return DynamicVector({kFirstFunction(tX, tY), kSecondFunction(tX, tY), kThirdFunction(tX, tY)});
        },
        [](const DynamicVector<double>& aX) { return make_jacobian_multiplier_two_to_three(aX); });
}
auto create_vector_function_three_to_two() -> VectorFunction
{
    return core::make_function(
        [](const DynamicVector<double>& aX)
        {
            const double tX = aX[0];
            const double tY = aX[1];
            const double tZ = aX[2];
            return DynamicVector({kForthFunction(tX, tY, tZ), kFifthFunction(tX, tY, tZ)});
        },
        [](const DynamicVector<double>& aX) { return make_jacobian_multiplier_three_to_two(aX); });
}

}  // namespace

TEST(LinearAlgebra, ComposeTwoVectorFunctions)
{
    const auto tF = create_vector_function_three_to_two();
    const auto tG = create_vector_function_two_to_three();
    // auto tDfOfG = [tF = std::move(aF), tG = std::move(aG)](const gArg& tX) { return tF.df(tG.f(tX)) * tG.df(tX); };
    const auto tCompositionFOfG = core::compose(tF, tG);

    const DynamicVector<double> tX({.4, .6});
    const DynamicVector<double> tDirection({-.1, .2});
    EXPECT_EQ(tF.f(tG.f(tX)).stdVector(), tCompositionFOfG.f(tX).stdVector());

    const auto tResult = tCompositionFOfG.df(tX).mJacobianTimesVectorFunction(tDirection);

    const auto tResultOfGDf = tG.df(tX).mJacobianTimesVectorFunction(tDirection);
    const auto tGold = tF.df(tG.f(tX)).mJacobianTimesVectorFunction(tResultOfGDf);

    ASSERT_EQ(tResult.size(), 2U);
    ASSERT_EQ(tGold.size(), 2U);
    EXPECT_DOUBLE_EQ(tResult.stdVector()[0], tGold.stdVector()[0]);
    EXPECT_DOUBLE_EQ(tResult.stdVector()[1], tGold.stdVector()[1]);
}

}  // namespace plato::linear_algebra::unittest
