#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/ConstraintValidation.hpp"
#include "plato/criteria/library/CriterionValidation.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::criteria::library::unittest
{
TEST(ConstraintValidation, ValidateConstraintValue)
{
    namespace pfcd = plato::criteria::library::detail;
    auto tConstraint = input_parser::new_constraint{};
    EXPECT_TRUE(pfcd::validate_constraint_value(tConstraint).has_value());
    tConstraint.constraint_value = 1.0;  // has only 1 : valid
    EXPECT_FALSE(pfcd::validate_constraint_value(tConstraint).has_value());
}

TEST(ConstraintValidation, ValidateConstraintType)
{
    namespace pfcd = plato::criteria::library::detail;

    auto tConstraint = input_parser::new_constraint{};
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
    auto tConstraint = input_parser::new_constraint{};
    EXPECT_FALSE(detail::validate_constraint_number_of_processors(tConstraint).has_value());
    tConstraint.number_of_processors = 42u;  // Only one is valid
    EXPECT_TRUE(detail::validate_constraint_number_of_processors(tConstraint).has_value());
    tConstraint.number_of_processors = 1u;
    EXPECT_FALSE(detail::validate_constraint_number_of_processors(tConstraint).has_value());
}

TEST(ConstraintValidation, ErrorMessagesInvalidConstraint)
{
    auto tConstraint = create_valid_example_constraint_input();
    tConstraint.app = boost::none;
    tConstraint.criterion = boost::none;
    const auto tMessages = input_validation::validate(tConstraint, {});
    EXPECT_EQ(tMessages.size(), 1u);
}

TEST(ConstraintValidation, ErrorMessagesTwoInvalidInput)
{
    namespace pfc = plato::criteria::library;
    input_parser::constraint tConstraint;
    auto tConstraintTwo = tConstraint;
    tConstraint.name = "bad-one";
    tConstraintTwo.name = "bad-two";
    const std::vector<input_parser::constraint> tInput{tConstraint, tConstraintTwo};

    const auto tMessages = pfc::validate_constraints(tInput, {});

    EXPECT_EQ(tMessages.size(), 6u);
}

TEST(ConstraintValidation, NoErrorMessagesTwoValidConstraints)
{
    namespace pfc = plato::criteria::library;
    const auto tConstraint = plato::test_utilities::create_valid_example_constraint();
    const std::vector<input_parser::constraint> tInput{tConstraint, tConstraint};

    const auto tMessages = pfc::validate_constraints(tInput, {});
    EXPECT_EQ(tMessages.size(), 0u);
}

TEST(ConstraintValidation, ErrorMessagesTwoInvalidConstraints)
{
    namespace pfc = plato::criteria::library;
    auto tConstraint = plato::test_utilities::create_valid_example_constraint();
    tConstraint.constraint_value = boost::none;
    auto tConstraintTwo = tConstraint;
    tConstraint.name = "bad-one";
    tConstraintTwo.name = "bad-two";
    const std::vector<input_parser::constraint> tInput{tConstraint, tConstraintTwo};

    const auto tMessages = pfc::validate_constraints(tInput, {});
    EXPECT_EQ(tMessages.size(), 2u);
}
}  // namespace plato::criteria::library::unittest
