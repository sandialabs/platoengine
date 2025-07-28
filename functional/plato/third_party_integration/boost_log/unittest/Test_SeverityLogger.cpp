#include <gtest/gtest.h>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/boost_log/ComponentAttributes.hpp"
#include "plato/third_party_integration/boost_log/InternalLoggerConsoleSink.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"
#include "plato/third_party_integration/boost_log/SeverityLogger.hpp"

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

    auto tLogger =
        SeverityLogger{ComponentTypeAndNameAttribute{ComponentTypeAndName{
                           .mComponentType = components::ComponentType::kFilter, .mComponentName = "helmholtz"}},
                       MPIWorldCommRankAttribute{}, LogSourceAttribute<LogSource::kInternal>{}};

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
    checkLogMessage(Severity::kDebug, std::mem_fn(&SeverityLogger::logDebugMessage), TEST_CONTEXT("Debug log member"));
    checkLogMessage(Severity::kInfo, std::mem_fn(&SeverityLogger::logInfo), TEST_CONTEXT("Info log member"));
    checkLogMessage(Severity::kWarning, std::mem_fn(&SeverityLogger::logWarning), TEST_CONTEXT("Warning log member"));
    checkLogMessage(Severity::kError, std::mem_fn(&SeverityLogger::logError), TEST_CONTEXT("Error log member"));
}

}  // namespace plato::third_party_integration::boost_log::unittest
