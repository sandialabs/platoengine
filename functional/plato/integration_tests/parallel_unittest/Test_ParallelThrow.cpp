#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <cstddef>
#include <stdexcept>

#include "plato/components/ComponentType.hpp"
#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"
#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerFactory.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::integration_tests::parallel_unittest
{
namespace
{
[[nodiscard]] auto non_throwing_objective(const std::size_t aNumberOfProcessors, const input_parser::AppName& aAppName)
{
    auto tObjective = input_parser::objective{};
    tObjective.number_of_processors = aNumberOfProcessors;
    tObjective.aggregation_weight = 1.0;
    tObjective.app = aAppName;
    tObjective.criterion = input_parser::CriterionName{"mass"};
    tObjective.name = "i_dont_throw";
    return tObjective;
}

void check_for_throw_on_all_ranks(const input_parser::ParsedInput& aInput,
                                  const test_utilities::TestContext& aTestContext)
{
    const auto tValidatedInputOrError = input_validation::make_validated_input(aInput);
    ASSERT_TRUE(tValidatedInputOrError.hasValue()) << tValidatedInputOrError.error();
    const auto& tValidatedInput = tValidatedInputOrError.value();

    const auto tProcessManager = process_manager::library::make_process_managers(
        tValidatedInput.get<components::ComponentType::kProcessManager>());
    ASSERT_EQ(tProcessManager.size(), 1U);
    const auto tROLOptimize = tProcessManager[0];
    const auto tProcessManagerData = process_manager::library::make_process_manager_data(tValidatedInput);

    // This can hang if the throw is not handled correctly in parallel execution
    EXPECT_THROW(tROLOptimize(tProcessManagerData), std::runtime_error) << aTestContext;
}
}  // namespace

TEST(ParallelThrow, AllRanksThrowOnExceptionForObjectives)
{
    const auto tAppName = input_parser::AppName{"test-mass-app"};
    [[maybe_unused]] const auto tConfigurationTempDirectory =
        utilities::register_test_mass_app(tAppName.mToken, boost::mpi::communicator{});

    const auto tThrowingObjective = [&tAppName]()
    {
        auto tObjective = input_parser::objective{};
        tObjective.number_of_processors = 1U;
        tObjective.aggregation_weight = 1.0;
        tObjective.app = tAppName;
        tObjective.criterion = input_parser::CriterionName{"runtime_throw"};
        tObjective.name = "i_throw";
        return tObjective;
    }();

    constexpr auto tNumberOfProcessors = std::size_t{3U};
    const auto tNonThrowingObjective = non_throwing_objective(tNumberOfProcessors, tAppName);

    const auto tInput = tThrowingObjective | tNonThrowingObjective |
                        geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                        process_manager::extension::test_utilities::create_valid_example_rol_optimization_input();

    check_for_throw_on_all_ranks(tInput, TEST_CONTEXT("Multiple objectives"));
}

TEST(ParallelThrow, AllRanksThrowOnExceptionForConstraints)
{
    const auto tAppName = input_parser::AppName{"test-mass-app"};
    [[maybe_unused]] const auto tConfigurationTempDirectory =
        utilities::register_test_mass_app(tAppName.mToken, boost::mpi::communicator{});

    const auto tThrowingConstraint = [&tAppName]()
    {
        auto tConstraint = input_parser::constraint{};
        tConstraint.number_of_processors = 1U;
        tConstraint.app = tAppName;
        tConstraint.criterion = input_parser::CriterionName{"runtime_throw"};
        tConstraint.name = "i_throw";
        tConstraint.is_linear = true;
        tConstraint.constraint_value = 0.0;
        tConstraint.constraint_type = input_parser::ConstraintTypes::kEqualTo;
        return tConstraint;
    }();
    constexpr auto tNumberOfProcessors = std::size_t{4U};
    const auto tNonThrowingObjective = non_throwing_objective(tNumberOfProcessors, tAppName);

    const auto tInput = tThrowingConstraint | tNonThrowingObjective |
                        geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                        process_manager::extension::test_utilities::create_valid_example_rol_optimization_input();

    check_for_throw_on_all_ranks(tInput, TEST_CONTEXT("Multiple objectives"));
}
}  // namespace plato::integration_tests::parallel_unittest
