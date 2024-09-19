#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/ValidInputTestFixture.hpp"

namespace plato::integration_tests::serial
{
namespace
{
struct ConstraintFactoryTestFixture : public test_utilities::ValidInputTestFixture
{
};

auto create_raw_input() -> input_parser::ParsedInput
{
    return test_utilities::create_valid_density_topology_geometry() | test_utilities::create_valid_example_objective() |
           test_utilities::create_valid_example_constraint() | test_utilities::create_valid_example_rol_optimization() |
           test_utilities::create_valid_identity_filter();
}

void test_constraint_type_and_value(const criteria::library::Constraint<const mesh::MeshDesignVariables&>& aConstraint,
                                    const double aGoldValue,
                                    const criteria::library::ConstraintType aType)
{
    EXPECT_TRUE(aConstraint.mLinear);
    EXPECT_EQ(aConstraint.mConstraintTarget, aGoldValue);
    EXPECT_EQ(aConstraint.mConstraintType, aType);
}

criteria::library::Constraint<const mesh::MeshDesignVariables&> get_first_constraint(
    const input_parser::ParsedInput& aInput)
{
    const auto tData = process_manager::library::make_validated_input(aInput);
    return criteria::library::detail::make_constraint(tData.constraints().rawInput().front());
}

}  // namespace

TEST_F(ConstraintFactoryTestFixture, ValidEqualityConstraint)
{
    const auto tRawInput = create_raw_input();

    test_constraint_type_and_value(get_first_constraint(tRawInput), 0.0, criteria::library::ConstraintType::kEquality);
}

TEST_F(ConstraintFactoryTestFixture, ValidInequalityConstraint)
{
    auto tRawInput = create_raw_input();
    {
        tRawInput.mConstraints[0].constraint_type = input_parser::ConstraintTypes::kLessThan;
        tRawInput.mConstraints[0].constraint_value = 1;
        test_constraint_type_and_value(get_first_constraint(tRawInput), 1.0,
                                       criteria::library::ConstraintType::kLessThan);
    }
    {
        tRawInput.mConstraints[0].constraint_type = input_parser::ConstraintTypes::kGreaterThan;
        tRawInput.mConstraints[0].constraint_value = 2;
        test_constraint_type_and_value(get_first_constraint(tRawInput), 2.0,
                                       criteria::library::ConstraintType::kGreaterThan);
    }
}

}  // namespace plato::integration_tests::serial
