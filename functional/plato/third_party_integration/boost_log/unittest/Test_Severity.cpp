#include <gtest/gtest.h>

#include <sstream>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"

namespace plato::third_party_integration::boost_log::unittest
{
TEST(Severity, StreamInsertion)
{
    const auto tExpectStreamMatches =
        [](const Severity aSeverity, const std::string_view aExpected, const test_utilities::TestContext& aTestContext)
    {
        auto tSeverityStream = std::stringstream{};
        tSeverityStream << aSeverity;
        EXPECT_EQ(tSeverityStream.view(), aExpected) << aTestContext;
    };

    tExpectStreamMatches(Severity::kDebug, "debug", TEST_CONTEXT("Debug"));
    tExpectStreamMatches(Severity::kInfo, "info", TEST_CONTEXT("Info"));
    tExpectStreamMatches(Severity::kWarning, "warning", TEST_CONTEXT("Warning"));
    tExpectStreamMatches(Severity::kError, "error", TEST_CONTEXT("Error"));
}
}  // namespace plato::third_party_integration::boost_log::unittest
