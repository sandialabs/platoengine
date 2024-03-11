#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/criteria/library/ConstraintValidation.hpp"
#include "plato/main/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::main::library::unittest
{
TEST(ConstraintFactory, MultipleValidConstraints)
{
    input_parser::ParsedInput tInput = plato::test_utilities::create_valid_example_input();
    input_parser::constraint tConstraint = plato::test_utilities::create_valid_example_constraint();
    tConstraint.name = "eq";
    tConstraint.app = input_parser::CodeOptions::kNodalSum;
    tConstraint.equal_to = 13;
    tInput.mConstraints = {tConstraint};

    tConstraint.name = "le";
    tConstraint.equal_to = 17;
    tConstraint.is_linear = false;
    tInput.mConstraints.push_back(tConstraint);

    tConstraint.name = "ge";
    tConstraint.equal_to = 10;
    tConstraint.is_linear = false;
    tInput.mConstraints.push_back(tConstraint);

    const ValidatedInput tData = make_validated_input(tInput);
    auto tCons = plato::criteria::library::make_constraints(tData.constraints());
    ASSERT_EQ(tCons.size(), 3);

    EXPECT_TRUE(tCons[0].mLinear);
    EXPECT_EQ(tCons[0].mConstraintTarget, 13);
    EXPECT_FALSE(tCons[1].mLinear);
    EXPECT_EQ(tCons[1].mConstraintTarget, 17);
    EXPECT_FALSE(tCons[2].mLinear);
    EXPECT_EQ(tCons[2].mConstraintTarget, 10);
}
}  // namespace plato::main::library::unittest
