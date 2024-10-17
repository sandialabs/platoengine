#include <gtest/gtest.h>

#include <cmath>

#include "plato/core/Compose.hpp"
#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::linear_algebra::unittest
{
using FunctionEvaluationInfo = core::FunctionInfo<DynamicVector<double>, core::evaluation::kFunction>;
using JacobianInfo = core::FunctionInfo<JacobianMultiplier, core::evaluation::kFirstDerivative>;
using AdjointJacobianInfo =
    core::FunctionInfo<AdjointJacobianMultiplier, core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>;
using VectorFunction =
    core::Function<const DynamicVector<double>&, FunctionEvaluationInfo, JacobianInfo, AdjointJacobianInfo>;

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
    const double tZ = aV[2];
    return DynamicVector({2.0 * tX, 2.0 * tY, 2.0 * tZ});
};

auto make_jacobian_multiplier_two_to_three(const DynamicVector<double>& aX) -> JacobianMultiplier
{
    const auto tFirstFunctionGrad = kFirstFunctionGrad(aX);
    const auto tSecondFunctionGrad = kSecondFunctionGrad(aX);
    const auto tThirdFunctionGrad = kThirdFunctionGrad(aX);

    const JacobianMultiplier::JacobianTimesVectorFunction tJacobianTimesVectorFunction =
        [tFirstFunctionGrad, tSecondFunctionGrad, tThirdFunctionGrad](const DynamicVector<double>& aDualVector)
    {
        return DynamicVector<double>({tFirstFunctionGrad[0] * aDualVector[0] + tSecondFunctionGrad[0] * aDualVector[1] +
                                          tThirdFunctionGrad[0] * aDualVector[2],
                                      tFirstFunctionGrad[1] * aDualVector[0] + tSecondFunctionGrad[1] * aDualVector[1] +
                                          tThirdFunctionGrad[1] * aDualVector[2]});
    };

    return {tJacobianTimesVectorFunction};
}

auto make_adjoint_jacobian_multiplier_two_to_three(const DynamicVector<double>& aX) -> AdjointJacobianMultiplier
{
    const auto tFirstFunctionGrad = kFirstFunctionGrad(aX);
    const auto tSecondFunctionGrad = kSecondFunctionGrad(aX);
    const auto tThirdFunctionGrad = kThirdFunctionGrad(aX);

    const JacobianMultiplier::JacobianTimesVectorFunction tJacobianTimesVectorFunction =
        [tFirstFunctionGrad, tSecondFunctionGrad, tThirdFunctionGrad](const DynamicVector<double>& aVector)
    {
        return DynamicVector<double>(
            {tFirstFunctionGrad.dot(aVector), tSecondFunctionGrad.dot(aVector), tThirdFunctionGrad.dot(aVector)});
    };

    return AdjointJacobianMultiplier{JacobianMultiplier{tJacobianTimesVectorFunction}};
}

auto make_jacobian_multiplier_three_to_one(const DynamicVector<double>& aX) -> JacobianMultiplier
{
    const auto tForthFunctionGrad = kForthFunctionGrad(aX);

    const JacobianMultiplier::JacobianTimesVectorFunction tJacobianTimesVectorFunction =
        [tForthFunctionGrad](const DynamicVector<double>& aDualVector)
    {
        return DynamicVector<double>({tForthFunctionGrad[0] * aDualVector[0], tForthFunctionGrad[1] * aDualVector[0],
                                      tForthFunctionGrad[2] * aDualVector[0]});
    };

    return {tJacobianTimesVectorFunction};
}

auto make_adjoint_jacobian_multiplier_three_to_one(const DynamicVector<double>& aX) -> AdjointJacobianMultiplier
{
    const auto tForthFunctionGrad = kForthFunctionGrad(aX);

    const JacobianMultiplier::JacobianTimesVectorFunction tJacobianTimesVectorFunction =
        [tForthFunctionGrad](const DynamicVector<double>& aVector)
    { return DynamicVector<double>({tForthFunctionGrad.dot(aVector)}); };

    return AdjointJacobianMultiplier{JacobianMultiplier{tJacobianTimesVectorFunction}};
}

auto create_vector_function_two_to_three() -> VectorFunction
{
    return VectorFunction{
        [](const DynamicVector<double>& aX)
        {
            const double tX = aX[0];
            const double tY = aX[1];
            return DynamicVector({kFirstFunction(tX, tY), kSecondFunction(tX, tY), kThirdFunction(tX, tY)});
        },
        [](const DynamicVector<double>& aX) { return make_jacobian_multiplier_two_to_three(aX); },
        [](const DynamicVector<double>& aX) { return make_adjoint_jacobian_multiplier_two_to_three(aX); }};
}

auto create_vector_function_three_to_one() -> VectorFunction
{
    return VectorFunction{[](const DynamicVector<double>& aX)
                          {
                              const double tX = aX[0];
                              const double tY = aX[1];
                              const double tZ = aX[2];
                              return DynamicVector({kForthFunction(tX, tY, tZ)});
                          },
                          [](const DynamicVector<double>& aX) { return make_jacobian_multiplier_three_to_one(aX); },
                          [](const DynamicVector<double>& aX)
                          { return make_adjoint_jacobian_multiplier_three_to_one(aX); }};
}

}  // namespace

TEST(LinearAlgebra, ComposeTwoVectorFunctions)
{
    const auto tF = create_vector_function_three_to_one();
    const auto tG = create_vector_function_two_to_three();

    const auto tCompositionFOfG = core::compose(tF, tG);

    const DynamicVector<double> tX{1.0, 2.0};
    // Function evaluation
    {
        EXPECT_EQ(tF.evaluate<0>(tG.evaluate<core::evaluation::kFunction>(tX)).stdVector(),
                  tCompositionFOfG.evaluate<0>(tX).stdVector());
    }
    // Jacobian
    {
        const DynamicVector<double> tDirection{-2.0};
        const auto tResult = tDirection * tCompositionFOfG.evaluate<core::evaluation::kFirstDerivative>(tX);
        const auto tVectorTimesFJacobian =
            tDirection * tF.evaluate<core::evaluation::kFirstDerivative>(tG.evaluate<core::evaluation::kFunction>(tX));
        const auto tExpected = tVectorTimesFJacobian * tG.evaluate<core::evaluation::kFirstDerivative>(tX);

        ASSERT_EQ(tResult.size(), 2U);
        ASSERT_EQ(tExpected.size(), 2U);
        EXPECT_EQ(tResult.stdVector(), tExpected.stdVector());
    }
    // Adjoint Jacobian
    {
        const auto tDual = DynamicVector{-3.0, 4.0};
        const auto tResult =
            tDual * tCompositionFOfG.evaluate<core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>(tX);
        const auto tExpectedFromMatlab = std::vector<double>{182.0};

        EXPECT_EQ(tResult.size(), 1U);
        EXPECT_EQ(tResult.stdVector(), tExpectedFromMatlab);
    }
}

TEST(LinearAlgebra, MakeAdjointJacobianMultiplierTwoToThree)
{
    const auto tX = DynamicVector{1.0, 2.0};
    const auto tAdjointJacobianMultiplier = make_adjoint_jacobian_multiplier_two_to_three(tX);
    const auto tDual = DynamicVector{3.0, -1.0};
    const auto tResult = tDual * tAdjointJacobianMultiplier;
    const auto tExpectedFromMatlab = std::vector{11.0, -1.0, 2.0};

    EXPECT_EQ(tResult.size(), 3U);
    EXPECT_EQ(tResult.stdVector(), tExpectedFromMatlab);
}

TEST(LinearAlgebra, MakeAdjointJacobianMultiplierThreeToOne)
{
    const auto tX = DynamicVector{1.0, 2.0, 3.0};
    const auto tAdjointJacobianMultiplier = make_adjoint_jacobian_multiplier_three_to_one(tX);
    const auto tDual = DynamicVector{-3.0, 2.0, -1.0};
    const auto tResult = tDual * tAdjointJacobianMultiplier;
    const auto tExpectedFromMatlab = std::vector{-4.0};

    EXPECT_EQ(tResult.size(), 1U);
    EXPECT_EQ(tResult.stdVector(), tExpectedFromMatlab);
}

}  // namespace plato::linear_algebra::unittest
