#include <gtest/gtest.h>

#include <boost/log/expressions.hpp>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"
#include "plato/third_party_integration/boost_log/SeverityLogger.hpp"
#include "plato/third_party_integration/boost_log/SinkWithAttributeFormattersAndFilters.hpp"
#include "plato/third_party_integration/boost_log/test_utilities/TestAttributes.hpp"

namespace plato::third_party_integration::boost_log::unittest
{
namespace
{
void checkLogMessage(const Severity aSeverity, const plato::test_utilities::TestContext& aTestContext)
{
    const auto tStream = std::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLogSink =
        sink_with_attribute_formatters_and_filters<test_utilities::SharkAttribute, SeverityAttribute>(tStream);

    constexpr auto kSharkAttributeValue = std::string_view{"hammerhead"};
    auto tLogger = SeverityLogger{test_utilities::SharkAttribute{std::string{kSharkAttributeValue}}};

    constexpr auto tMessage = std::string_view{"Shark week!"};
    tLogger.logMessage(tMessage, aSeverity);

    EXPECT_NE(tStream->str().find(kSharkAttributeValue), std::string::npos)
        << aTestContext << "Result: " << tStream->str();
    EXPECT_NE(tStream->str().find(tMessage), std::string::npos) << aTestContext << "Result: " << tStream->str();

    auto tSeverityAsString = std::stringstream{};
    tSeverityAsString << aSeverity;
    EXPECT_NE(tStream->str().find(tSeverityAsString.str()), std::string::npos)
        << aTestContext << "Result: " << tStream->str();
}
}  // namespace

TEST(SeverityLogger, LogMembers)
{
    checkLogMessage(Severity::kDebug, TEST_CONTEXT("Debug log member"));
    checkLogMessage(Severity::kInfo, TEST_CONTEXT("Info log member"));
    checkLogMessage(Severity::kWarning, TEST_CONTEXT("Warning log member"));
    checkLogMessage(Severity::kError, TEST_CONTEXT("Error log member"));
}

}  // namespace plato::third_party_integration::boost_log::unittest
