#include <gtest/gtest.h>

#include <cstddef>
#include <vector>

#include "plato/components/ComponentType.hpp"
#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/process_manager/extension/ObjectiveGradientCheck.hpp"
#include "plato/process_manager/extension/ROLOptimization.hpp"
#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerFactory.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::process_manager::extension::unittest
{
template <typename BlockType>
[[nodiscard]] auto num_blocks_with_type(
    const std::vector<input_validation::ValidatedInputDataBlock<components::ComponentType::kProcessManager>>&
        aAllProcessManagerInputs) -> std::size_t
{
    return std::count_if(aAllProcessManagerInputs.cbegin(), aAllProcessManagerInputs.cend(), [](const auto& aInput)
                         { return aInput.rawInput().mBlockName == input_parser::block_name<BlockType>(); });
}

TEST(ObjectiveGradientCheck, CreateGradientCheckRun)
{
    const auto tCheckGradientCheckRuns =
        [](const input_parser::ParsedInput& aParsedInput, const plato::test_utilities::TestContext& aTestContext)
    {
        const auto tValidatedInput = input_validation::make_validated_input(aParsedInput);
        ASSERT_TRUE(tValidatedInput.hasValue());
        const auto tProblem = library::make_process_manager_data(tValidatedInput.value());
        const auto tAllProcessManagerInputs = tValidatedInput.value().get<components::ComponentType::kProcessManager>();
        ASSERT_EQ(tAllProcessManagerInputs.rawInput().size(), 1U);
        const auto tProcessManagers = library::make_process_managers(tAllProcessManagerInputs);
        ASSERT_EQ(tProcessManagers.size(), 1U);
        tProcessManagers.front()(tProblem);
        const auto tGradientCheckFilePath = aParsedInput.get<components::ComponentType::kProcessManager>()
                                                .front()
                                                .mInput.get<input_parser::objective_gradient_check>()
                                                .output_file_name;

        plato::test_utilities::test_for_existence_and_remove(
            {tGradientCheckFilePath.value().mToken}, EXTEND_CONTEXT("Checking for file existence", aTestContext));
    };

    const auto tBaseInput = geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                            criteria::library::test_utilities::create_valid_example_objective_input() |
                            test_utilities::create_valid_example_gradient_check_input();

    {
        tCheckGradientCheckRuns(tBaseInput, TEST_CONTEXT("No constraints"));
    }
    {
        auto tInequalityConstraint = criteria::library::test_utilities::create_valid_example_constraint_input();
        tInequalityConstraint.constraint_type = input_parser::ConstraintTypes::kLessThan;
        const auto tParsedInput = input_parser::ParsedInput{tBaseInput} | tInequalityConstraint;
        tCheckGradientCheckRuns(tParsedInput, TEST_CONTEXT("Inequality constraints"));
    }
}

TEST(ObjectiveGradientCheck, UnwrapValidatedGradientCheckInput)
{
    const auto tInputDeck = geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                            criteria::library::test_utilities::create_valid_example_objective_input() |
                            test_utilities::create_valid_example_rol_optimization_input() |
                            test_utilities::create_valid_example_gradient_check_input();

    const auto tValidatedInput = input_validation::make_validated_input(tInputDeck);
    const auto tUnwrappedValidatedInput =
        tValidatedInput.value().get<components::ComponentType::kProcessManager>().rawInput();

    constexpr auto tExpectedNumGradientCheckInputs = std::size_t{1};
    EXPECT_EQ(num_blocks_with_type<input_parser::objective_gradient_check>(tUnwrappedValidatedInput),
              tExpectedNumGradientCheckInputs);

    constexpr auto tExpectedNumROLOptimizerInputs = std::size_t{1};
    EXPECT_EQ(num_blocks_with_type<input_parser::rol_optimization>(tUnwrappedValidatedInput),
              tExpectedNumROLOptimizerInputs);

    constexpr auto tExpectedTotalProcessManagerInputs = std::size_t{2};
    EXPECT_EQ(tUnwrappedValidatedInput.size(), tExpectedTotalProcessManagerInputs);
}

TEST(ObjectiveGradientCheck, Registration)
{
    EXPECT_TRUE(library::is_process_manager_function_registered("objective_gradient_check"));
}

}  // namespace plato::process_manager::extension::unittest
