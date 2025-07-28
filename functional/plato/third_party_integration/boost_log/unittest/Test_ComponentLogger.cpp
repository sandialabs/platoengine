#include <gtest/gtest.h>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/boost_log/ComponentLogger.hpp"
#include "plato/third_party_integration/boost_log/InternalLoggerConsoleSink.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"

namespace plato::third_party_integration::boost_log::unittest
{
namespace
{
constexpr auto kFilterName = std::string_view{"helmholtz"};

void checkLogMessage(const Severity aSeverity,
                     const auto& aLogMemberFunction,
                     const plato::test_utilities::TestContext& aTestContext)
{
    const auto tStream = std::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLogSink = internal_console_sink(tStream);

    auto tLogger = ComponentLogger{components::ComponentType::kFilter, kFilterName};

    constexpr auto tMessage = std::string_view{"Evaluating filter."};
    aLogMemberFunction(tLogger, tMessage);

    EXPECT_NE(tStream->str().find("filter"), std::string::npos) << aTestContext << "Result: " << tStream->str();
    EXPECT_NE(tStream->str().find("helmholtz"), std::string::npos) << aTestContext << "Result: " << tStream->str();
    EXPECT_NE(tStream->str().find(tMessage), std::string::npos) << aTestContext << "Result: " << tStream->str();

    auto tSeverityAsString = std::stringstream{};
    tSeverityAsString << aSeverity;
    EXPECT_NE(tStream->str().find(tSeverityAsString.str()), std::string::npos)
        << aTestContext << "Result: " << tStream->str();
}
}  // namespace

TEST(ComponentLogger, LogMembers)
{
    checkLogMessage(Severity::kDebug, std::mem_fn(&ComponentLogger::logDebugMessage), TEST_CONTEXT("Debug log member"));
    checkLogMessage(Severity::kInfo, std::mem_fn(&ComponentLogger::logInfo), TEST_CONTEXT("Info log member"));
    checkLogMessage(Severity::kWarning, std::mem_fn(&ComponentLogger::logWarning), TEST_CONTEXT("Warning log member"));
    checkLogMessage(Severity::kError, std::mem_fn(&ComponentLogger::logError), TEST_CONTEXT("Error log member"));
}

}  // namespace plato::third_party_integration::boost_log::unittest
