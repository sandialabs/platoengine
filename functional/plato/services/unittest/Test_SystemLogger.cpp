#include <gtest/gtest.h>

#include "plato/services/InternalLoggerConsoleSink.hpp"
#include "plato/services/SystemLogger.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"

namespace plato::services::unittest
{
namespace
{
constexpr auto kFilterName = std::string_view{"helmholtz"};

void checkLogMessage(SystemLogger& aSystemLogger,
                     const third_party_integration::boost_log::Severity aSeverity,
                     const auto& aLogMemberFunction,
                     const plato::test_utilities::TestContext& aTestContext)
{
    const auto tStream = boost::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLogSink = internal_logger_console_sink(tStream);

    constexpr auto tMessage = std::string_view{"Evaluating filter."};
    aLogMemberFunction(aSystemLogger, tMessage);

    EXPECT_NE(tStream->str().find(tMessage), std::string::npos) << aTestContext << "Result: " << tStream->str();

    auto tSeverityAsString = std::stringstream{};
    tSeverityAsString << aSeverity;
    EXPECT_NE(tStream->str().find(tSeverityAsString.str()), std::string::npos)
        << aTestContext << "Result: " << tStream->str();
}

void checkLogMessageComponentAttributes(SystemLogger& aSystemLogger,
                                        const auto& aLogMemberFunction,
                                        const plato::test_utilities::TestContext& aTestContext)
{
    const auto tStream = boost::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLogSink = internal_logger_console_sink(tStream);

    aLogMemberFunction(aSystemLogger, "Evaluating filter.");

    EXPECT_NE(tStream->str().find("filter"), std::string::npos) << aTestContext << "Result: " << tStream->str();
    EXPECT_NE(tStream->str().find("helmholtz"), std::string::npos) << aTestContext << "Result: " << tStream->str();
}
}  // namespace

TEST(SystemLogger, ComponentLogMembers)
{
    auto tLogger = component_logger(components::ComponentType::kFilter, kFilterName);

    checkLogMessage(tLogger, third_party_integration::boost_log::Severity::kDebug,
                    std::mem_fn(&SystemLogger::logDebugMessage), TEST_CONTEXT("Debug log member"));
    checkLogMessage(tLogger, third_party_integration::boost_log::Severity::kInfo, std::mem_fn(&SystemLogger::logInfo),
                    TEST_CONTEXT("Info log member"));
    checkLogMessage(tLogger, third_party_integration::boost_log::Severity::kWarning,
                    std::mem_fn(&SystemLogger::logWarning), TEST_CONTEXT("Warning log member"));
    checkLogMessage(tLogger, third_party_integration::boost_log::Severity::kError, std::mem_fn(&SystemLogger::logError),
                    TEST_CONTEXT("Error log member"));

    checkLogMessageComponentAttributes(tLogger, std::mem_fn(&SystemLogger::logDebugMessage),
                                       TEST_CONTEXT("Debug log member"));
    checkLogMessageComponentAttributes(tLogger, std::mem_fn(&SystemLogger::logInfo), TEST_CONTEXT("Info log member"));
    checkLogMessageComponentAttributes(tLogger, std::mem_fn(&SystemLogger::logWarning),
                                       TEST_CONTEXT("Warning log member"));
    checkLogMessageComponentAttributes(tLogger, std::mem_fn(&SystemLogger::logError), TEST_CONTEXT("Error log member"));
}

TEST(SystemLogger, SystemLogMembers)
{
    auto tLogger = system_logger();

    checkLogMessage(tLogger, third_party_integration::boost_log::Severity::kDebug,
                    std::mem_fn(&SystemLogger::logDebugMessage), TEST_CONTEXT("Debug log member"));
    checkLogMessage(tLogger, third_party_integration::boost_log::Severity::kInfo, std::mem_fn(&SystemLogger::logInfo),
                    TEST_CONTEXT("Info log member"));
    checkLogMessage(tLogger, third_party_integration::boost_log::Severity::kWarning,
                    std::mem_fn(&SystemLogger::logWarning), TEST_CONTEXT("Warning log member"));
    checkLogMessage(tLogger, third_party_integration::boost_log::Severity::kError, std::mem_fn(&SystemLogger::logError),
                    TEST_CONTEXT("Error log member"));
}

}  // namespace plato::services::unittest
