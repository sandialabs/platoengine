#ifndef PLATO_INTEGRATION_TESTS_UTILITIES_CHECKPROCESSORSMATCHOBJECTIVES
#define PLATO_INTEGRATION_TESTS_UTILITIES_CHECKPROCESSORSMATCHOBJECTIVES

#include <vector>

#include "plato/criteria/library/ObjectiveFactory.hpp"

namespace plato::test_utilities
{
struct TestContext;
}

namespace plato::integration_tests::utilities
{
/// @brief Uses gtest macros to check that @a aNumberOfProcessors and @a aObjectives have the
/// same length and that each entry in @a aNumberOfProcessors matches the `number_of_processors`
/// field in @a aObjectives.
void check_processors_match_objectives(const std::vector<unsigned int>& aNumberOfProcessors,
                                       const criteria::library::ValidatedObjectives& aObjectives,
                                       const test_utilities::TestContext& aTestContext);
}  // namespace plato::integration_tests::utilities

#endif
