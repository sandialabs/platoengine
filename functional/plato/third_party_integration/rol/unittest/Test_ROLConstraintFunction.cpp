#include <gtest/gtest.h>

#include <ROL_StdVector.hpp>
#include <memory>
#include <vector>

#include "plato/third_party_integration/rol/ROLConstraintFunction.hpp"
#include "plato/third_party_integration/rol/Utilities.hpp"
#include "plato/third_party_integration/rol/unittest/DynamicVectorConstraintFunctions.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::third_party_integration::rol::unittest
{

namespace
{

constexpr double kValue = 5;

}  // namespace

TEST(ROLConstraintFunction, ConstraintValue)
{
    namespace pft = plato::test_utilities;
    pft::Himmelblau tHimmelblau{};

    const double tControlValue = 0.5;
    const std::vector<double> tGold = {tHimmelblau.f(tControlValue, tControlValue) - kValue};
    constexpr bool tIsLinear = false;

    auto tROLConstraintFunction =
        ROLConstraintFunction{criteria::library::Constraint<const linear_algebra::DynamicVector<double>&>{
            "name", make_himmelblau_dynamic_vector_function(pft::Himmelblau{}), kValue, tIsLinear,
            criteria::library::ConstraintType::kEquality}};
    const auto tControls = std::vector<double>{tControlValue, tControlValue};
    auto tConstraintsVector = std::vector<double>{.0};
    double tTolerance;

    tROLConstraintFunction.value(tConstraintsVector, tControls, tTolerance);
    EXPECT_EQ(tGold, tConstraintsVector);
}

TEST(ROLConstraintFunction, JacobianTimesDirection)
{
    namespace pft = plato::test_utilities;
    pft::Himmelblau tHimmelblau{};

    const double tControlValue = 0.5;
    const auto tGoldVec = tHimmelblau.df(tControlValue, tControlValue);
    auto tDirection = std::vector<double>{1.0, -1.0};
    const double tGoldValue = (tGoldVec.mData[0] - tGoldVec.mData[1]);

    constexpr bool tIsLinear = true;
    auto tROLConstraintFunction =
        ROLConstraintFunction{criteria::library::Constraint<const linear_algebra::DynamicVector<double>&>{
            "name", make_himmelblau_dynamic_vector_function(pft::Himmelblau{}), kValue, tIsLinear,
            criteria::library::ConstraintType::kEquality}};

    const auto tControls = std::vector<double>{tControlValue, tControlValue};
    std::vector<double> tJacobianTimesDirection{1.0};

    double tTolerance;
    tROLConstraintFunction.applyJacobian(tJacobianTimesDirection, tDirection, tControls, tTolerance);
    ASSERT_EQ(tJacobianTimesDirection.size(), 1u);

    const double tResult = tJacobianTimesDirection.front();
    EXPECT_EQ(tGoldValue, tResult);
}

TEST(ROLConstraintFunction, AdjointJacobianTimesDirection)
{
    namespace pft = plato::test_utilities;
    pft::Himmelblau tHimmelblau{};

    constexpr std::string_view tConstraintName = "con name";

    const double tControlValue = 0.5;
    auto tGold = tHimmelblau.df(tControlValue, tControlValue);
    const double tDual = 1.2;
    const std::vector<double> tGoldVec{tGold.mData[0] * tDual, tGold.mData[1] * tDual};

    auto tDirection = std::vector<double>{tDual};
    constexpr bool tIsLinear = true;
    auto tROLConstraintFunction =
        ROLConstraintFunction{criteria::library::Constraint<const linear_algebra::DynamicVector<double>&>{
            std::string{tConstraintName}, make_himmelblau_dynamic_vector_function(pft::Himmelblau{}), kValue, tIsLinear,
            criteria::library::ConstraintType::kEquality}};

    const auto tControls = std::vector<double>{tControlValue, tControlValue};
    std::vector<double> tAdjointJacobianTimesDirection{1.0, 1.0};

    double tTolerance;

    tROLConstraintFunction.applyAdjointJacobian(tAdjointJacobianTimesDirection, tDirection, tControls, tTolerance);
    ASSERT_EQ(tAdjointJacobianTimesDirection.size(), 2u);

    EXPECT_EQ(tGoldVec, tAdjointJacobianTimesDirection);
}

TEST(ROLConstraintFunction, Hessian)
{
    namespace pft = plato::test_utilities;

    constexpr double tTarget = 5;
    constexpr double tControlValue = 0.5;
    constexpr bool tIsLinear = false;
    auto tROLConstraintFunction =
        ROLConstraintFunction{criteria::library::Constraint<const linear_algebra::DynamicVector<double>&>{
            "name", make_himmelblau_dynamic_vector_function(pft::Himmelblau{}), tTarget, tIsLinear}};

    const auto tControls = std::vector<double>{tControlValue, tControlValue};
    auto tHessianUV = std::vector<double>{10, 11};
    const auto tDirection = std::vector<double>{1, 1};
    double tTolerance;
    tROLConstraintFunction.applyAdjointHessian(tHessianUV, tDirection, tDirection, tControls, tTolerance);

    EXPECT_EQ(tHessianUV[0], 0.0);
    EXPECT_EQ(tHessianUV[1], 0.0);
}

}  // namespace plato::third_party_integration::rol::unittest
