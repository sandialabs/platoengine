#include <gtest/gtest.h>

#include <ROL_StdVector.hpp>
#include <vector>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/rol/ROLConstraint.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::third_party_integration::rol::unittest
{

namespace
{

ROL::Ptr<ROL::StdVector<double>> to_rol_std_vector_ptr(const std::vector<double>& aVector)
{
    auto tVector = ROL::makePtr<std::vector<double>>(aVector);
    return ROL::makePtr<ROL::StdVector<double>>(tVector);
}

void test_bounds_behaves_same_as_gold(const ROL::Ptr<ROL::Bounds<double>>& aBounds,
                                      const ROL::Ptr<ROL::Bounds<double>>& aGoldBounds)
{
    auto tNegativeOne = to_rol_std_vector_ptr({-1});
    auto tZero = to_rol_std_vector_ptr({0});
    auto tPositiveOne = to_rol_std_vector_ptr({1});
    EXPECT_EQ(aBounds->isFeasible(*tNegativeOne), aGoldBounds->isFeasible(*tNegativeOne));
    EXPECT_EQ(aBounds->isFeasible(*tZero), aGoldBounds->isFeasible(*tZero));
    EXPECT_EQ(aBounds->isFeasible(*tPositiveOne), aGoldBounds->isFeasible(*tPositiveOne));
}

}  // namespace

TEST(ROLConstraint, CreateROLBoundConstraint)
{
    const std::vector<double> tLower(2, 0);
    const std::vector<double> tUpper{1, 2};
    const auto tBounds = create_rol_bound_constraint({tLower, tUpper});
    auto tCompare = to_rol_std_vector_ptr({0.4, 0.5});
    EXPECT_TRUE(tBounds->isFeasible(*tCompare));
    tCompare = to_rol_std_vector_ptr({-0.4, 0.5});
    EXPECT_FALSE(tBounds->isFeasible(*tCompare));
    tCompare = to_rol_std_vector_ptr({0.4, -0.5});
    EXPECT_FALSE(tBounds->isFeasible(*tCompare));
    tCompare = to_rol_std_vector_ptr({4, -5});
    EXPECT_FALSE(tBounds->isFeasible(*tCompare));
}

TEST(ROLConstraint, SizedDualVector)
{
    constexpr unsigned int tSize = 10;
    const linear_algebra::DynamicVector<double> tDualVector = make_dual_vector(tSize);
    ASSERT_EQ(tDualVector.size(), tSize);
    EXPECT_EQ(tDualVector.stdVector().front(), 1.0);
    EXPECT_EQ(tDualVector.stdVector().back(), 1.0);
}

TEST(ROLConstraint, CreateLessThanInequalityBounds)
{
    EXPECT_THROW([[maybe_unused]] auto tUnused = detail::create_less_than_inequality_bounds(0), utilities::Exception);
    {
        constexpr unsigned int tNumberOfConstraints = 1;
        const auto tBounds = detail::create_less_than_inequality_bounds(tNumberOfConstraints);

        auto tCompare = to_rol_std_vector_ptr({-1});
        EXPECT_TRUE(tBounds->isFeasible(*tCompare));
        tCompare = to_rol_std_vector_ptr({1});
        EXPECT_FALSE(tBounds->isFeasible(*tCompare));
    }
    {
        constexpr unsigned int tNumberOfConstraints = 2;
        const auto tBounds = detail::create_less_than_inequality_bounds(tNumberOfConstraints);

        auto tCompare = to_rol_std_vector_ptr({-1, -2});
        EXPECT_TRUE(tBounds->isFeasible(*tCompare));
        tCompare = to_rol_std_vector_ptr({-1});
        EXPECT_ANY_THROW(tBounds->isFeasible(*tCompare));
    }
}

TEST(ROLConstraint, CreateGreaterThanInequalityBounds)
{
    EXPECT_THROW([[maybe_unused]] auto tUnused = detail::create_greater_than_inequality_bounds(0),
                 utilities::Exception);
    {
        constexpr unsigned int tNumberOfConstraints = 1;
        const auto tBounds = detail::create_greater_than_inequality_bounds(tNumberOfConstraints);

        auto tCompare = to_rol_std_vector_ptr({1});
        EXPECT_TRUE(tBounds->isFeasible(*tCompare));
        tCompare = to_rol_std_vector_ptr({-1});
        EXPECT_FALSE(tBounds->isFeasible(*tCompare));
    }
    {
        constexpr unsigned int tNumberOfConstraints = 2;
        const auto tBounds = detail::create_greater_than_inequality_bounds(tNumberOfConstraints);

        auto tCompare = to_rol_std_vector_ptr({1, 2});
        EXPECT_TRUE(tBounds->isFeasible(*tCompare));
        tCompare = to_rol_std_vector_ptr({-1});
        EXPECT_ANY_THROW(tBounds->isFeasible(*tCompare));
    }
}

TEST(ROLConstraint, CreateInequalityBounds)
{
    constexpr unsigned int tNumberOfConstraints = 1;
    {
        const auto tGold = detail::create_greater_than_inequality_bounds(tNumberOfConstraints);
        const auto tResult =
            detail::create_inequality_bounds(criteria::library::ConstraintType::kGreaterThan, tNumberOfConstraints);
        test_bounds_behaves_same_as_gold(tResult, tGold);
    }
    {
        const auto tGold = detail::create_less_than_inequality_bounds(tNumberOfConstraints);
        const auto tResult =
            detail::create_inequality_bounds(criteria::library::ConstraintType::kLessThan, tNumberOfConstraints);
        test_bounds_behaves_same_as_gold(tResult, tGold);
    }
    EXPECT_THROW([[maybe_unused]] auto tUnused = detail::create_inequality_bounds(
                     criteria::library::ConstraintType::kEqualTo, tNumberOfConstraints),
                 std::out_of_range);
}

TEST(ROLConstraint, ConstraintCombination)
{
    const auto tCheckConstraintCombination = [](const ROLConstraint& aConstraint,
                                                const detail::ConstraintCombination aExpectedConstraintCombination,
                                                const test_utilities::TestContext& aTestContext)
    {
        const auto tConstraintCombination = detail::constraint_combination(aConstraint);
        EXPECT_EQ(tConstraintCombination, aExpectedConstraintCombination) << aTestContext;
    };

    auto tConstraint = ROLConstraint{};

    tConstraint.mLinear = true;
    tConstraint.mType = criteria::library::ConstraintType::kEqualTo;
    tCheckConstraintCombination(tConstraint, detail::ConstraintCombination::kLinearEquality,
                                TEST_CONTEXT("Linear equality"));

    tConstraint.mType = criteria::library::ConstraintType::kGreaterThan;
    tCheckConstraintCombination(tConstraint, detail::ConstraintCombination::kLinearInequality,
                                TEST_CONTEXT("Linear inequality with greater than"));

    tConstraint.mType = criteria::library::ConstraintType::kLessThan;
    tCheckConstraintCombination(tConstraint, detail::ConstraintCombination::kLinearInequality,
                                TEST_CONTEXT("Linear inequality with less than"));

    tConstraint.mLinear = false;
    tConstraint.mType = criteria::library::ConstraintType::kEqualTo;
    tCheckConstraintCombination(tConstraint, detail::ConstraintCombination::kNonlinearEquality,
                                TEST_CONTEXT("Non-linear equality"));

    tConstraint.mType = criteria::library::ConstraintType::kGreaterThan;
    tCheckConstraintCombination(tConstraint, detail::ConstraintCombination::kNonlinearInequality,
                                TEST_CONTEXT("Non-linear inequality with greater than"));

    tConstraint.mType = criteria::library::ConstraintType::kLessThan;
    tCheckConstraintCombination(tConstraint, detail::ConstraintCombination::kNonlinearInequality,
                                TEST_CONTEXT("Non-linear inequality with less than"));
}

}  // namespace plato::third_party_integration::rol::unittest
