#include <gtest/gtest.h>

#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerFactory.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::process_manager::snopt::unittest_missing_snopt
{
TEST(SNOPTMissingOptimization, UnregisteredProcessManagers)
{
    const auto tInputDeck = test_utilities::create_valid_brick_shape_geometry() |
                            test_utilities::create_valid_example_objective() |
                            test_utilities::create_valid_example_snopt_optimization();
    const auto tValidatedInput = library::make_validated_input(tInputDeck);
    const auto tUnregisteredProcessManagers = library::unregistered_process_managers(tValidatedInput.processManagers());
    const auto tExpected = std::vector<std::string>{"snopt_optimization"};
    EXPECT_EQ(tUnregisteredProcessManagers, tExpected);
}
}  // namespace plato::process_manager::snopt::unittest_missing_snopt
