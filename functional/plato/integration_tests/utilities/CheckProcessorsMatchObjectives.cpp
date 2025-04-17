#include "plato/integration_tests/utilities/CheckProcessorsMatchObjectives.hpp"

#include <gtest/gtest.h>

#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::integration_tests::utilities
{
void check_processors_match_objectives(const std::vector<unsigned int>& aNumberOfProcessors,
                                       const criteria::library::NewValidatedObjectives& aObjectives,
                                       const test_utilities::TestContext& aTestContext)
{
    for (const auto [tNumberOfProcessors, tObjective] :
         plato::utilities::Zip{aNumberOfProcessors, aObjectives.rawInput()})
    {
        const auto tObjectiveInput = input_validation::get_input_block<input_parser::new_objective>(tObjective);
        if (tObjectiveInput.active.value_or(true))
        {
            EXPECT_EQ(tNumberOfProcessors, tObjectiveInput.number_of_processors.value_or(1U)) << aTestContext;
        }
    }
}
}  // namespace plato::integration_tests::utilities
