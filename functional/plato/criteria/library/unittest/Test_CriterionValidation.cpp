#include <gtest/gtest.h>

#include "plato/criteria/library/CriterionValidation.hpp"
#include "plato/input_parser/InputBlocks.hpp"

namespace plato::criteria::library::unittest
{
namespace
{
template <typename Criteria>
void check_validation_app_and_custom_app()
{
    namespace pfcd = plato::criteria::library::detail;
    Criteria tCriteria;
    EXPECT_TRUE(pfcd::validate_app(tCriteria).has_value());
    tCriteria.app = input_parser::CodeOptions::kCustomApp;
    EXPECT_FALSE(pfcd::validate_app(tCriteria).has_value());
    EXPECT_TRUE(pfcd::validate_custom_app(tCriteria).has_value());
    tCriteria.shared_library_path = input_parser::FileName{"/sweet/potato/ravioli.so"};
    EXPECT_FALSE(pfcd::validate_custom_app(tCriteria).has_value());

    EXPECT_FALSE(pfcd::validate_number_of_processors(tCriteria).has_value());
    tCriteria.number_of_processors = 0;
    EXPECT_TRUE(pfcd::validate_number_of_processors(tCriteria).has_value());
    tCriteria.number_of_processors = 1;
    EXPECT_FALSE(pfcd::validate_number_of_processors(tCriteria).has_value());
}

}  // namespace

TEST(CriterionValidation, CheckValidationAppAndCustomAppOnObjective)
{
    check_validation_app_and_custom_app<input_parser::objective>();
}

TEST(CriterionValidation, CheckValidationAppAndCustomAppOnConstraint)
{
    check_validation_app_and_custom_app<input_parser::constraint>();
}
}  // namespace plato::criteria::library::unittest
