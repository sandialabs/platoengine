#include <gtest/gtest.h>

#include <ROL_StdVector.hpp>
#include <memory>
#include <vector>

#include "plato/third_party_integration/rol/ROLConstraintFunction.hpp"
#include "plato/third_party_integration/rol/unittest/DynamicVectorHimmelblauFunction.hpp"

namespace plato::third_party_integration::rol::unittest
{
TEST(ROLConstraintFunction, ConstraintValue)
{
    namespace pft = plato::test_utilities;

    pft::Himmelblau tHimmelblau{};

    constexpr double tTarget = 5;

    const double tControlValue = 0.5;
    const std::vector<double> tGold = {tHimmelblau.f(tControlValue, tControlValue) - tTarget};

    constexpr bool tIsLinear = false;
    auto tROLConstraintFunction =
        ROLConstraintFunction{criteria::library::Constraint<const linear_algebra::DynamicVector<double>&>{
            "name", make_himmelblau_dynamic_vector_function(pft::Himmelblau{}), tTarget, tIsLinear}};
    const auto tControls = ROL::StdVector<double>{tControlValue, tControlValue};
    auto tConstraintsVector = ROL::StdVector<double>{.0};
    double tTolerance;

    tROLConstraintFunction.value(tConstraintsVector, tControls, tTolerance);
    EXPECT_EQ(tGold, *tConstraintsVector.getVector());
    EXPECT_FALSE(tROLConstraintFunction.linear());
}

TEST(ROLConstraintFunction, JacobianTimesDirection)
{
    namespace pft = plato::test_utilities;

    pft::Himmelblau tHimmelblau{};

    constexpr double tTarget = 5;

    const double tControlValue = 0.5;
    const auto tGoldVec = tHimmelblau.df(tControlValue, tControlValue);
    auto tDirection = ROL::StdVector<double>{1.0, -1.0};
    const double tGoldValue = (tGoldVec.mData[0] - tGoldVec.mData[1]);

    constexpr bool tIsLinear = true;
    auto tROLConstraintFunction =
        ROLConstraintFunction{criteria::library::Constraint<const linear_algebra::DynamicVector<double>&>{
            "name", make_himmelblau_dynamic_vector_function(pft::Himmelblau{}), tTarget, tIsLinear}};

    const auto tControls = ROL::StdVector<double>{tControlValue, tControlValue};
    ROL::StdVector<double> tJacobianTimesDirection{1.0};

    double tTolerance;

    tROLConstraintFunction.applyJacobian(tJacobianTimesDirection, tDirection, tControls, tTolerance);
    ASSERT_EQ(tJacobianTimesDirection.getVector()->size(), 1u);

    const double tResult = tJacobianTimesDirection.getVector()->front();
    EXPECT_EQ(tGoldValue, tResult);
    EXPECT_TRUE(tROLConstraintFunction.linear());
}

TEST(ROLConstraintFunction, AdjointJacobianTimesDirection)
{
    namespace pft = plato::test_utilities;

    pft::Himmelblau tHimmelblau{};

    constexpr double tTarget = 5;
    constexpr std::string_view tConstraintName = "con name";

    const double tControlValue = 0.5;
    auto tGold = tHimmelblau.df(tControlValue, tControlValue);
    const double tDual = 1.2;
    const std::vector<double> tGoldVec{tGold.mData[0] * tDual, tGold.mData[1] * tDual};

    auto tDirection = ROL::StdVector<double>{tDual};
    constexpr bool tIsLinear = true;
    auto tROLConstraintFunction =
        ROLConstraintFunction{criteria::library::Constraint<const linear_algebra::DynamicVector<double>&>{
            std::string{tConstraintName}, make_himmelblau_dynamic_vector_function(pft::Himmelblau{}), tTarget,
            tIsLinear}};

    const auto tControls = ROL::StdVector<double>{tControlValue, tControlValue};
    ROL::StdVector<double> tAdjointJacobianTimesDirection{1.0, 1.0};

    double tTolerance;

    tROLConstraintFunction.applyAdjointJacobian(tAdjointJacobianTimesDirection, tDirection, tControls, tTolerance);
    ASSERT_EQ(tAdjointJacobianTimesDirection.getVector()->size(), 2u);

    const auto tResult = *tAdjointJacobianTimesDirection.getVector();
    EXPECT_EQ(tGoldVec, tResult);
    EXPECT_TRUE(tROLConstraintFunction.linear());
    EXPECT_EQ(tROLConstraintFunction.name(), std::string{tConstraintName});
}

}  // namespace plato::third_party_integration::rol::unittest
