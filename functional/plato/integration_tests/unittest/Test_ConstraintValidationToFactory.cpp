#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/criteria/library/ConstraintValidation.hpp"
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
    auto tConstraint = criteria::library::create_valid_example_constraint_input();
    tConstraint.name = "eq";
    tConstraint.constraint_value = 13;

    auto tInput = parsedInput();
    tInput = tInput | tConstraint;

    tConstraint.name = "le";
    tConstraint.constraint_value = 17;
    tConstraint.is_linear = false;
    tInput = tInput | tConstraint;

    tConstraint.name = "ge";
    tConstraint.constraint_value = 10;
    tConstraint.is_linear = false;
    tInput = tInput | tConstraint;

    const auto tData = input_validation::make_validated_input(tInput).value();
    auto tConstraints =
        plato::criteria::library::make_constraints(tData.get<input_parser::ComponentType::kConstraint>());
    ASSERT_EQ(tConstraints.size(), 3u);

    EXPECT_TRUE(tConstraints[0].mLinear);
    EXPECT_EQ(tConstraints[0].mConstraintTarget, 13);
    EXPECT_FALSE(tConstraints[1].mLinear);
    EXPECT_EQ(tConstraints[1].mConstraintTarget, 17);
    EXPECT_FALSE(tConstraints[2].mLinear);
    EXPECT_EQ(tConstraints[2].mConstraintTarget, 10);
}
}  // namespace plato::integration_tests::unittest
