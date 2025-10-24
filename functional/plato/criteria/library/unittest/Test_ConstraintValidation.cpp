#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/ConstraintValidation.hpp"

namespace plato::criteria::library::unittest
{
TEST(ConstraintValidation, ValidateConstraintValue)
{
    namespace pfcd = plato::criteria::library::detail;
    // Valid, only constraint_value is set
    {
        auto tConstraint = input_parser::constraint{};
        tConstraint.constraint_value = 1.0;  // has only 1 : valid
        EXPECT_FALSE(pfcd::validate_constraint_value(tConstraint).has_value());
    }
    // Valid, only constraint_value_list is set
    {
        auto tConstraint = input_parser::constraint{};
        tConstraint.constraint_value_list = ConstraintValueList{{input_parser::ComponentAndTarget{}}};
        EXPECT_FALSE(pfcd::validate_constraint_value(tConstraint).has_value());
    }
    // Invalid, no constraint targets
    {
        auto tConstraint = input_parser::constraint{};
        EXPECT_TRUE(pfcd::validate_constraint_value(tConstraint).has_value());
    }
    // Invalid, both constraint target types are set (constraint_value and constraint_value_list)
    {
        auto tConstraint = input_parser::constraint{};
        tConstraint.constraint_value = 1.0;
        tConstraint.constraint_value_list = ConstraintValueList{{input_parser::ComponentAndTarget{}}};
        EXPECT_TRUE(pfcd::validate_constraint_value(tConstraint).has_value());
    }
}

TEST(ConstraintValidation, ValidateConstraintType)
{
    namespace pfcd = plato::criteria::library::detail;

    auto tConstraint = input_parser::constraint{};
    EXPECT_TRUE(pfcd::validate_constraint_type(tConstraint).has_value());

    tConstraint.constraint_type = input_parser::ConstraintTypes::kEqualTo;
    EXPECT_FALSE(pfcd::validate_constraint_type(tConstraint).has_value());

    tConstraint.constraint_type = input_parser::ConstraintTypes::kLessThan;
    EXPECT_FALSE(pfcd::validate_constraint_type(tConstraint).has_value());

    tConstraint.constraint_type = input_parser::ConstraintTypes::kGreaterThan;
    EXPECT_FALSE(pfcd::validate_constraint_type(tConstraint).has_value());
}

TEST(ConstraintValidation, NumberOfProcessors)
{
    auto tConstraint = input_parser::constraint{};
    EXPECT_FALSE(detail::validate_constraint_number_of_processors(tConstraint).has_value());
    tConstraint.number_of_processors = 42u;  // Only one is valid
    EXPECT_TRUE(detail::validate_constraint_number_of_processors(tConstraint).has_value());
    tConstraint.number_of_processors = 1u;
    EXPECT_FALSE(detail::validate_constraint_number_of_processors(tConstraint).has_value());
}

}  // namespace plato::criteria::library::unittest
