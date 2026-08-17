#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <stdexcept>

#include "plato/components/ComponentType.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"
#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerFactory.hpp"

namespace plato::integration_tests::parallel_unittest
{
TEST(ParallelThrow, AllRanksThrowOnException)
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

    const auto tNonThrowingObjective = [&tAppName]()
    {
        auto tObjective = input_parser::objective{};
        tObjective.number_of_processors = 3U;
        tObjective.aggregation_weight = 1.0;
        tObjective.app = tAppName;
        tObjective.criterion = input_parser::CriterionName{"mass"};
        tObjective.name = "i_dont_throw";
        return tObjective;
    }();

    const auto tInput = tThrowingObjective | tNonThrowingObjective |
                        geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                        process_manager::extension::test_utilities::create_valid_example_rol_optimization_input();

    const auto tValidatedInputOrError = input_validation::make_validated_input(tInput);
    ASSERT_TRUE(tValidatedInputOrError.hasValue()) << tValidatedInputOrError.error();
    const auto& tValidatedInput = tValidatedInputOrError.value();

    const auto tProcessManager = process_manager::library::make_process_managers(
        tValidatedInput.get<components::ComponentType::kProcessManager>());
    ASSERT_EQ(tProcessManager.size(), 1U);
    const auto tROLOptimize = tProcessManager[0];
    const auto tProcessManagerData = process_manager::library::make_process_manager_data(tValidatedInput);

    // This can hang if the throw is not handled correctly in parallel execution
    EXPECT_THROW(tROLOptimize(tProcessManagerData), std::runtime_error);
}
}  // namespace plato::integration_tests::parallel_unittest
