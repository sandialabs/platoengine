#include <gtest/gtest.h>

#include "plato/criteria/library/CriterionValidation.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_parser/InputDefinitions.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::criteria::library::unittest
{
namespace
{
template <typename Criteria>
void check_validation_app(const test_utilities::TestContext& aTestContext)
{
    Criteria tCriteria;
    EXPECT_TRUE(detail::validate_criterion_is_registered(tCriteria).has_value()) << aTestContext;

    tCriteria.app = input_parser::AppName{"definitely not an app"};
    EXPECT_TRUE(detail::validate_criterion_is_registered(tCriteria).has_value()) << aTestContext;

    EXPECT_FALSE(detail::validate_number_of_processors(tCriteria).has_value()) << aTestContext;
    tCriteria.number_of_processors = 0;
    EXPECT_TRUE(detail::validate_number_of_processors(tCriteria).has_value()) << aTestContext;
    tCriteria.number_of_processors = 1;
    EXPECT_FALSE(detail::validate_number_of_processors(tCriteria).has_value()) << aTestContext;
}

}  // namespace

TEST(CriterionValidation, CheckValidationAppAndCustomAppOnObjective)
{
    check_validation_app<input_parser::objective>(TEST_CONTEXT("Objective"));
}

TEST(CriterionValidation, CheckValidationAppAndCustomAppOnConstraint)
{
    check_validation_app<input_parser::constraint>(TEST_CONTEXT("Constraint"));
}
}  // namespace plato::criteria::library::unittest
