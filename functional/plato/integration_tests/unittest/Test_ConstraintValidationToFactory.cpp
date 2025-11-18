#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/integration_tests/utilities/ValidInputTestFixture.hpp"

namespace plato::integration_tests::unittest
{
namespace
{
struct ConstraintFactoryFileFixture : public utilities::ValidInputTestFixture
{
};
}  // namespace

TEST_F(ConstraintFactoryFileFixture, MultipleValidConstraints)
{
    auto tInput = parsedInput();
    tInput.get<components::ComponentType::kConstraint>().clear();

    auto tConstraint = criteria::library::test_utilities::create_valid_example_constraint_input();
    tConstraint.name = "eq";
    tConstraint.constraint_value = 13;
    tConstraint.constraint_type = input_parser::ConstraintTypes::kEqualTo;
    tInput = tInput | tConstraint;

    tConstraint.name = "le";
    tConstraint.constraint_value = 17;
    tConstraint.is_linear = false;
    tConstraint.constraint_type = input_parser::ConstraintTypes::kGreaterThan;
    tInput = tInput | tConstraint;

    tConstraint.name = "ge";
    tConstraint.constraint_value = 10;
    tConstraint.is_linear = false;
    tConstraint.constraint_type = input_parser::ConstraintTypes::kLessThan;
    tInput = tInput | tConstraint;

    const auto tData = input_validation::make_validated_input(tInput).value();
    auto tConstraints = plato::criteria::library::make_constraints(tData.get<components::ComponentType::kConstraint>());
    ASSERT_EQ(tConstraints.size(), 3U);

    EXPECT_TRUE(tConstraints[0].mLinear);
    EXPECT_EQ(tConstraints[0].mConstraintType, criteria::library::ConstraintType::kEqualTo);
    EXPECT_FALSE(tConstraints[1].mLinear);
    EXPECT_EQ(tConstraints[1].mConstraintType, criteria::library::ConstraintType::kGreaterThan);
    EXPECT_FALSE(tConstraints[2].mLinear);
    EXPECT_EQ(tConstraints[2].mConstraintType, criteria::library::ConstraintType::kLessThan);
}
}  // namespace plato::integration_tests::unittest
