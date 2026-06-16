#include <gtest/gtest.h>

#include <boost/smart_ptr/make_shared_object.hpp>
#include <functional>
#include <sstream>

#include "plato/components/ComponentType.hpp"
#include "plato/services/InternalLoggerConsoleSink.hpp"
#include "plato/services/SystemLogger.hpp"
#include "plato/test_utilities/Strings.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"

namespace plato::services::unittest
{
namespace
{
constexpr auto kFilterName = std::string_view{"helmholtz"};

void check_log_message(SystemLogger& aSystemLogger,
                       const third_party_integration::boost_log::Severity aSeverity,
                       const auto& aLogMemberFunction,
                       const plato::test_utilities::TestContext& aTestContext)
{
    const auto tStream = boost::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLogSink = internal_logger_console_sink(tStream);

    constexpr auto tMessage = std::string_view{"Evaluating filter."};
    aLogMemberFunction(aSystemLogger, tMessage);

    test_utilities::expect_string_contains_substring(tStream->str(), tMessage, aTestContext);
    test_utilities::expect_string_contains_substring(tStream->str(), to_string(aSeverity), aTestContext);
}

void check_log_message_component_attributes(SystemLogger& aSystemLogger,
                                            const auto& aLogMemberFunction,
                                            const plato::test_utilities::TestContext& aTestContext)
{
    const auto tStream = boost::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLogSink = internal_logger_console_sink(tStream);

    aLogMemberFunction(aSystemLogger, "Evaluating filter.");

    test_utilities::expect_string_contains_substring(tStream->str(), "filter", aTestContext);
    test_utilities::expect_string_contains_substring(tStream->str(), "helmholtz", aTestContext);
}

void check_all_log_members(SystemLogger& aLogger, const test_utilities::TestContext& aTestContext)
{
    check_log_message(aLogger, third_party_integration::boost_log::Severity::kDebug,
                      std::mem_fn(&SystemLogger::logDebugMessage), EXTEND_CONTEXT("Debug log member", aTestContext));
    check_log_message(aLogger, third_party_integration::boost_log::Severity::kInfo, std::mem_fn(&SystemLogger::logInfo),
                      EXTEND_CONTEXT("Info log member", aTestContext));
    check_log_message(aLogger, third_party_integration::boost_log::Severity::kWarning,
                      std::mem_fn(&SystemLogger::logWarning), EXTEND_CONTEXT("Warning log member", aTestContext));
    check_log_message(aLogger, third_party_integration::boost_log::Severity::kError,
                      std::mem_fn(&SystemLogger::logError), EXTEND_CONTEXT("Error log member", aTestContext));
}
}  // namespace

TEST(SystemLogger, ComponentLogMembers)
{
    auto tLogger = component_logger(components::ComponentType::kFilter, kFilterName);

    check_all_log_members(tLogger, TEST_CONTEXT("Component logger"));

    check_log_message_component_attributes(tLogger, std::mem_fn(&SystemLogger::logDebugMessage),
                                           TEST_CONTEXT("Debug log member"));
    check_log_message_component_attributes(tLogger, std::mem_fn(&SystemLogger::logInfo),
                                           TEST_CONTEXT("Info log member"));
    check_log_message_component_attributes(tLogger, std::mem_fn(&SystemLogger::logWarning),
                                           TEST_CONTEXT("Warning log member"));
    check_log_message_component_attributes(tLogger, std::mem_fn(&SystemLogger::logError),
                                           TEST_CONTEXT("Error log member"));
}

TEST(SystemLogger, SystemLogMembers)
{
    auto tLogger = system_logger();

    check_all_log_members(tLogger, TEST_CONTEXT("Component logger"));
}

}  // namespace plato::services::unittest
