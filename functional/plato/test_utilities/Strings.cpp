#include "plato/test_utilities/Strings.hpp"

#include <gtest/gtest.h>

namespace plato::test_utilities
{
void expect_string_contains_substring(const std::string& aStringUnderTest,
                                      std::string_view aStringToFind,
                                      const TestContext& aTestContext)
{
    EXPECT_NE(aStringUnderTest.find(aStringToFind), std::string::npos)
        << aTestContext << " String: " << aStringUnderTest << " does not contain " << aStringToFind;
}

void expect_string_does_not_contain_substring(const std::string& aStringUnderTest,
                                              std::string_view aStringToFind,
                                              const TestContext& aTestContext)
{
    EXPECT_EQ(aStringUnderTest.find(aStringToFind), std::string::npos)
        << aTestContext << " String: " << aStringUnderTest << " does not contain " << aStringToFind;
}

}  // namespace plato::test_utilities
