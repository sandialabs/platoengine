#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/filter/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/integration_tests/utilities/ValidInputTestFixture.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::integration_tests::serial
{
namespace
{
struct ConstraintFactoryTestFixture : public utilities::ValidInputTestFixture
{
};

void test_constraint_type_and_value(
    const criteria::library::VectorConstraint<const analysis::AnalysisDomainMesh&>& aConstraint,
    const double aGoldValue,
    const criteria::library::ConstraintType aType,
    const test_utilities::TestContext& aTestContext)
{
    EXPECT_TRUE(aConstraint.mLinear) << aTestContext;
    EXPECT_EQ(aConstraint.mConstraintTarget, aGoldValue) << aTestContext;
    EXPECT_EQ(aConstraint.mConstraintType, aType) << aTestContext;
}

[[nodiscard]] auto get_first_constraint(const input_parser::ParsedInput& aInput)
{
    const auto tData = input_validation::make_validated_input(aInput).value();
    return criteria::library::detail::make_constraint(
        tData.get<components::ComponentType::kConstraint>().rawInput().front());
}

}  // namespace

TEST_F(ConstraintFactoryTestFixture, ValidEqualityConstraint)
{
    auto tInputBase = parsedInput();
    tInputBase.template get<components::ComponentType::kConstraint>().clear();
    const auto tRawInput = tInputBase | criteria::library::test_utilities::create_valid_example_constraint_input();

    test_constraint_type_and_value(get_first_constraint(tRawInput), 0.0, criteria::library::ConstraintType::kEqualTo,
                                   TEST_CONTEXT("Equality constraint"));
}

TEST_F(ConstraintFactoryTestFixture, ValidInequalityConstraint)
{
    auto tInputBase = parsedInput();
    tInputBase.template get<components::ComponentType::kConstraint>().clear();
    {
        constexpr auto tConstraintValue = 1.0;
        auto tConstraintInput = criteria::library::test_utilities::create_valid_example_constraint_input();
        tConstraintInput.constraint_type = input_parser::ConstraintTypes::kLessThan;
        tConstraintInput.constraint_value = tConstraintValue;
        auto tRawInput = tInputBase | tConstraintInput;
        test_constraint_type_and_value(get_first_constraint(tRawInput), tConstraintValue,
                                       criteria::library::ConstraintType::kLessThan, TEST_CONTEXT("Less than"));
    }
    {
        constexpr auto tConstraintValue = 2.0;
        auto tConstraintInput = criteria::library::test_utilities::create_valid_example_constraint_input();
        tConstraintInput.constraint_type = input_parser::ConstraintTypes::kGreaterThan;
        tConstraintInput.constraint_value = tConstraintValue;
        auto tRawInput = tInputBase | tConstraintInput;
        test_constraint_type_and_value(get_first_constraint(tRawInput), tConstraintValue,
                                       criteria::library::ConstraintType::kGreaterThan, TEST_CONTEXT("Greater than"));
    }
}

}  // namespace plato::integration_tests::serial
