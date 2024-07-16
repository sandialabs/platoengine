#include <gtest/gtest.h>

#include "plato/criteria/extension/NodalSumObjective.hpp"
#include "plato/criteria/library/CriterionValidation.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_parser/InputDefinitions.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::criteria::extension::unittest
{
namespace
{
template <typename Criteria>
void check_validation_app(const test_utilities::TestContext& aTestContext)
{
    Criteria tCriteria;
    tCriteria.app = input_parser::AppName{std::string{input_parser::kBuiltinAppName}};
    tCriteria.criterion = input_parser::CriterionName{std::string{NodalSumObjective::kCriterionName}};
    EXPECT_FALSE(library::detail::validate_criterion_is_registered(tCriteria).has_value()) << aTestContext;

    tCriteria.app = boost::none;
    EXPECT_FALSE(library::detail::validate_criterion_is_registered(tCriteria).has_value()) << aTestContext;

    tCriteria.app = input_parser::AppName{std::string{input_parser::kBuiltinAppName}};
    tCriteria.criterion = input_parser::CriterionName{"definitely not a criterion"};
    EXPECT_TRUE(library::detail::validate_criterion_is_registered(tCriteria).has_value()) << aTestContext;

    tCriteria.app = boost::none;
    EXPECT_TRUE(library::detail::validate_criterion_is_registered(tCriteria).has_value()) << aTestContext;
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
}  // namespace plato::criteria::extension::unittest
