#include <gtest/gtest.h>

#include <ROL_StdVector.hpp>
#include <vector>

#include "plato/test_utilities/Himmelblau.hpp"
#include "plato/third_party_integration/rol/ROLVectorConstraintFunction.hpp"
#include "plato/third_party_integration/rol/unittest/DynamicVectorConstraintFunctions.hpp"

namespace plato::third_party_integration::rol::unittest
{
namespace
{

constexpr double kValue = 5;
using VectorFunction = core::Function<linear_algebra::DynamicVector<double>,
                                      linear_algebra::JacobianMultiplier,
                                      const linear_algebra::DynamicVector<double>&>;
auto make_vector_himmelblau_adjoint_jacobian() -> VectorFunction
{
    auto tHimmelblau = make_himmelblau_dynamic_vector_function(test_utilities::Himmelblau{});

    return VectorFunction{[tHimmelblau](const linear_algebra::DynamicVector<double>& aFunctionArg)
                          { return linear_algebra::DynamicVector<double>{tHimmelblau.f(aFunctionArg)}; },
                          [tHimmelblau](const linear_algebra::DynamicVector<double>& aFunctionArg)
                          {
                              constexpr unsigned int tNumberOfColumns = 1;
                              const linear_algebra::DynamicVector<double> tDf = tHimmelblau.df(aFunctionArg);
                              const linear_algebra::JacobianMultiplier::JacobianTimesVectorFunction tFunction =
                                  [tDf](const linear_algebra::DynamicVector<double>& aV)
                              { return tDf * aV.stdVector()[0]; };
                              return linear_algebra::JacobianMultiplier{tNumberOfColumns, tFunction};
                          }};
}

auto make_vector_himmelblau_jacobian() -> VectorFunction
{
    auto tHimmelblau = make_himmelblau_dynamic_vector_function(test_utilities::Himmelblau{});

    return VectorFunction{[tHimmelblau](const linear_algebra::DynamicVector<double>& aFunctionArg)
                          { return linear_algebra::DynamicVector<double>{tHimmelblau.f(aFunctionArg)}; },
                          [tHimmelblau](const linear_algebra::DynamicVector<double>& aFunctionArg)
                          {
                              constexpr unsigned int tNumberOfColumns = 1;
                              const linear_algebra::DynamicVector<double> tDf = tHimmelblau.df(aFunctionArg);
                              const linear_algebra::JacobianMultiplier::JacobianTimesVectorFunction tFunction =
                                  [tDf](const linear_algebra::DynamicVector<double>& aV)
                              { return linear_algebra::DynamicVector<double>{tDf.dot(aV)}; };
                              return linear_algebra::JacobianMultiplier{tNumberOfColumns, tFunction};
                          }};
}

auto make_himmelblau_rol_vector_constraint() -> ROLVectorConstraintFunction
{
    const auto tVectorHimmelblauJacobian = make_vector_himmelblau_jacobian();
    const auto tVectorHimmelblauAdjointJacobian = make_vector_himmelblau_adjoint_jacobian();
    constexpr bool tIsLinear = false;
    return ROLVectorConstraintFunction{
        criteria::library::VectorConstraint<const linear_algebra::DynamicVector<double>&>{
            "name", tVectorHimmelblauJacobian, tVectorHimmelblauAdjointJacobian, kValue, tIsLinear,
            criteria::library::ConstraintType::kEquality}};
}

}  // namespace

TEST(ROLVectorConstraintFunction, Value)
{
    const auto tVectorHimmelblau = make_vector_himmelblau_jacobian();
    const auto tControlPoint = std::vector{.5, .20};
    const auto tDynamicVectorControlPoint = linear_algebra::DynamicVector<double>(tControlPoint);
    const std::vector<double> tGold =
        (tVectorHimmelblau.f(tDynamicVectorControlPoint) + linear_algebra::DynamicVector<double>(1, -1.0 * kValue))
            .stdVector();

    auto tROLVectorConstraintFunction = make_himmelblau_rol_vector_constraint();

    auto tConstraintsVector = std::vector<double>{.0};
    double tTolerance;
    tROLVectorConstraintFunction.value(tConstraintsVector, tControlPoint, tTolerance);

    ASSERT_EQ(tConstraintsVector.size(), tGold.size());
    EXPECT_EQ(tConstraintsVector.size(), 1u);
    EXPECT_EQ(tConstraintsVector[0], tGold[0]);
}

TEST(ROLVectorConstraintFunction, JacobianAndAdjointConsistency)
{
    const auto tDirectionVector = std::vector{.3, .6};
    const auto tDynamicVectorDirectionVector = linear_algebra::DynamicVector<double>(tDirectionVector);

    const auto tControlPoint = std::vector{.5, .20};
    const auto tDynamicVectorControlPoint = linear_algebra::DynamicVector<double>(tControlPoint);

    const auto tDualPoint = std::vector{.25};
    const auto tDynamicVectorDualPoint = linear_algebra::DynamicVector<double>(tDualPoint);

    auto tJacobianTimesVector = std::vector<double>{.0, .0};
    auto tAdjointJacobianTimesVector = std::vector<double>{.0, .0};
    double tTolerance;

    auto tROLVectorConstraintFunction = make_himmelblau_rol_vector_constraint();
    tROLVectorConstraintFunction.applyJacobian(tJacobianTimesVector, tDirectionVector, tControlPoint, tTolerance);
    tROLVectorConstraintFunction.applyAdjointJacobian(tAdjointJacobianTimesVector, tDualPoint, tControlPoint,
                                                      tTolerance);

    ASSERT_EQ(tJacobianTimesVector.size(), 1U);
    ASSERT_EQ(tAdjointJacobianTimesVector.size(), 2U);

    const auto tResultOne = (tDynamicVectorDualPoint * tJacobianTimesVector[0]).stdVector();
    const auto tResultTwo =
        linear_algebra::DynamicVector<double>{tAdjointJacobianTimesVector}.dot(tDynamicVectorDirectionVector);

    ASSERT_EQ(tResultOne.size(), 1U);

    EXPECT_EQ(tResultOne[0], tResultTwo);
}

}  // namespace plato::third_party_integration::rol::unittest
