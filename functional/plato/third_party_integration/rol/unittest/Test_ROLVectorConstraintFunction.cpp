#include <gtest/gtest.h>

#include <ROL_StdVector.hpp>
#include <vector>

#include "plato/core/Compose.hpp"
#include "plato/criteria/library/TargetOffsetFunction.hpp"
#include "plato/test_utilities/Himmelblau.hpp"
#include "plato/third_party_integration/rol/ROLVectorConstraintFunction.hpp"
#include "plato/third_party_integration/rol/unittest/DynamicVectorConstraintFunctions.hpp"

namespace plato::third_party_integration::rol::unittest
{
namespace
{
constexpr double kValue = 5;
using VectorFunction = ROLVectorConstraintFunction::ROLPlatoFunction;

auto make_vector_himmelblau_with_adjoint_function() -> VectorFunction
{
    const auto tHimmelblau = make_himmelblau_dynamic_vector_function(test_utilities::Himmelblau{});

    return VectorFunction{
        [tHimmelblau](const linear_algebra::DynamicVector<double>& aFunctionArg) {
            return linear_algebra::DynamicVector<double>{
                tHimmelblau.evaluate<core::evaluation::kFunction>(aFunctionArg)};
        },
        [tHimmelblau](const linear_algebra::DynamicVector<double>& aFunctionArg)
        {
            auto tFunction = [tDf = tHimmelblau.evaluate<core::evaluation::kFirstDerivative>(aFunctionArg)](
                                 const linear_algebra::DynamicVector<double>& aV) { return tDf * aV.stdVector()[0]; };
            return linear_algebra::JacobianMultiplier{std::move(tFunction)};
        },
        [tHimmelblau](const linear_algebra::DynamicVector<double>& aFunctionArg)
        {
            auto tFunction = [tDf = tHimmelblau.evaluate<core::evaluation::kFirstDerivative>(aFunctionArg)](
                                 const linear_algebra::DynamicVector<double>& aV)
            { return linear_algebra::DynamicVector{tDf.dot(aV)}; };
            return linear_algebra::AdjointJacobianMultiplier{linear_algebra::JacobianMultiplier{std::move(tFunction)}};
        }};
}

auto make_himmelblau_rol_vector_constraint() -> ROLVectorConstraintFunction
{
    constexpr bool tIsLinear = false;
    return ROLVectorConstraintFunction{
        criteria::library::VectorConstraint<const linear_algebra::DynamicVector<double>&>{
            .mName = "name",
            .mConstraintFunction = core::compose(criteria::library::make_target_offset_function(kValue),
                                                 make_vector_himmelblau_with_adjoint_function()),
            .mLinear = tIsLinear,
            .mConstraintType = criteria::library::ConstraintType::kEqualTo}};
}

}  // namespace

TEST(ROLVectorConstraintFunction, Value)
{
    const auto tVectorHimmelblau = make_vector_himmelblau_with_adjoint_function();
    const auto tControlPoint = linear_algebra::DynamicVector<double>{0.5, 0.2};
    const auto tGold = (tVectorHimmelblau.evaluate<core::evaluation::kFunction>(tControlPoint) +
                        linear_algebra::DynamicVector<double>(1, -1.0 * kValue))
                           .stdVector();

    auto tROLVectorConstraintFunction = make_himmelblau_rol_vector_constraint();

    auto tConstraintsVector = std::vector<double>{.0};
    double tTolerance;
    tROLVectorConstraintFunction.value(tConstraintsVector, tControlPoint.stdVector(), tTolerance);

    ASSERT_EQ(tConstraintsVector.size(), tGold.size());
    EXPECT_EQ(tConstraintsVector.size(), 1u);
    EXPECT_EQ(tConstraintsVector[0], tGold[0]);
}

TEST(ROLVectorConstraintFunction, JacobianAndAdjointConsistency)
{
    auto tROLVectorConstraintFunction = make_himmelblau_rol_vector_constraint();
    const auto tControlPoint = linear_algebra::DynamicVector{0.5, 0.2};
    double tTolerance;
    const auto tHimmelblau = make_himmelblau_dynamic_vector_function(test_utilities::Himmelblau{});
    const auto tGradientOfHimmelblau = tHimmelblau.evaluate<core::evaluation::kFirstDerivative>(tControlPoint);

    // Check Jacobian
    {
        const auto tDirectionVector = linear_algebra::DynamicVector{0.3, 0.6};
        auto tJacobianTimesVector = std::vector<double>{};

        tROLVectorConstraintFunction.applyJacobian(tJacobianTimesVector, tDirectionVector.stdVector(),
                                                   tControlPoint.stdVector(), tTolerance);
        const auto tExpected = tDirectionVector.dot(tGradientOfHimmelblau);

        ASSERT_EQ(tJacobianTimesVector.size(), 1U);
        EXPECT_EQ(tExpected, tJacobianTimesVector.front());
    }
    // Check adjoint Jacobian
    {
        const auto tDual = linear_algebra::DynamicVector{0.25};
        auto tAdjointJacobianTimesVector = std::vector<double>{};
        tROLVectorConstraintFunction.applyAdjointJacobian(tAdjointJacobianTimesVector, tDual.stdVector(),
                                                          tControlPoint.stdVector(), tTolerance);
        const auto tExpected = tDual[0] * tGradientOfHimmelblau;

        EXPECT_EQ(tExpected.stdVector(), tAdjointJacobianTimesVector);
    }
}

TEST(ROLVectorConstraintFunction, Hessian)
{
    auto tROLVectorConstraintFunction = make_himmelblau_rol_vector_constraint();

    constexpr auto tArbitraryControlValue = 10.0;
    const auto tControls = std::vector<double>{tArbitraryControlValue, tArbitraryControlValue};
    auto tHessianUV = std::vector{10.0, 11.0};
    const auto tDirection = std::vector{1.0, 1.0};
    double tTolerance;
    tROLVectorConstraintFunction.applyAdjointHessian(tHessianUV, tDirection, tDirection, tControls, tTolerance);

    EXPECT_EQ(tHessianUV[0], 0.0);
    EXPECT_EQ(tHessianUV[1], 0.0);
}

}  // namespace plato::third_party_integration::rol::unittest
