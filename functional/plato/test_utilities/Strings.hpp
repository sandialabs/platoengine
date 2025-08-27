#ifndef PLATO_TEST_UTILITIES_STRINGS
#define PLATO_TEST_UTILITIES_STRINGS

#include <string>
#include <string_view>

#include "plato/test_utilities/TestContext.hpp"

namespace plato::test_utilities
{
/// @brief Checks that @a aStringUnderTest contains as a full substring @a aStringToFind.
void expect_string_contains_substring(const std::string& aStringUnderTest,
                                      std::string_view aStringToFind,
                                      const TestContext& aTestContext);

/// @brief Checks that @a aStringUnderTest does not contain as a full substring @a aStringToFind.
void expect_string_does_not_contain_substring(const std::string& aStringUnderTest,
                                              std::string_view aStringToFind,
                                              const TestContext& aTestContext);

}  // namespace plato::test_utilities

#endif
