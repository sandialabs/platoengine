#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintValidation.hpp"
#include "plato/criteria/library/CriterionValidation.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::criteria::library::unittest
{
TEST(ConstraintValidation, ValidateEqualTo)
{
    namespace pfcd = plato::criteria::library::detail;
    input_parser::constraint tConstraint;
    EXPECT_TRUE(pfcd::validate_equal_to(tConstraint).has_value());
    tConstraint.equal_to = 1.0;  // has only 1 : valid
    EXPECT_FALSE(pfcd::validate_equal_to(tConstraint).has_value());
}

TEST(ConstraintValidation, ErrorMessagesInvalidConstraint)
{
    input_parser::constraint tConstraint = plato::test_utilities::create_valid_example_constraint();
    tConstraint.app = boost::none;
    std::vector<std::string> tMessages;
    tMessages = core::validate(tConstraint, std::move(tMessages));
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

    std::vector<std::string> tMessages;
    tMessages = pfc::validate_constraints(tInput, std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 4u);
}

TEST(ConstraintValidation, NoErrorMessagesTwoValidConstraints)
{
    namespace pfc = plato::criteria::library;
    const auto tConstraint = plato::test_utilities::create_valid_example_constraint();
    const std::vector<input_parser::constraint> tInput{tConstraint, tConstraint};

    std::vector<std::string> tMessages;
    tMessages = pfc::validate_constraints(tInput, std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 0u);
}

TEST(ConstraintValidation, ErrorMessagesTwoInvalidConstraints)
{
    namespace pfc = plato::criteria::library;
    auto tConstraint = plato::test_utilities::create_valid_example_constraint();
    tConstraint.equal_to = boost::none;
    auto tConstraintTwo = tConstraint;
    tConstraint.name = "bad-one";
    tConstraintTwo.name = "bad-two";
    const std::vector<input_parser::constraint> tInput{tConstraint, tConstraintTwo};

    std::vector<std::string> tMessages;
    tMessages = pfc::validate_constraints(tInput, std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 2u);
}
}  // namespace plato::criteria::library::unittest
