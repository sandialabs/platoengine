#include <gtest/gtest.h>

#include <boost/smart_ptr/make_shared_object.hpp>
#include <sstream>

#include "plato/test_utilities/Strings.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"
#include "plato/third_party_integration/boost_log/SeverityLogger.hpp"
#include "plato/third_party_integration/boost_log/SinkWithAttributeFormattersAndFilters.hpp"
#include "plato/third_party_integration/boost_log/test_utilities/TestAttributes.hpp"

namespace plato::third_party_integration::boost_log::unittest
{
namespace
{
void check_log_message(const Severity aSeverity, const plato::test_utilities::TestContext& aTestContext)
{
    const auto tStream = boost::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLogSink =
        sink_with_attribute_formatters_and_filters<test_utilities::SharkAttribute, SeverityAttribute>(tStream);

    constexpr auto tSharkAttributeValue = std::string_view{"hammerhead"};
    auto tLogger = SeverityLogger{test_utilities::SharkAttribute{std::string{tSharkAttributeValue}}};

    constexpr auto tMessage = std::string_view{"Shark week!"};
    tLogger.logMessage(tMessage, aSeverity);

    plato::test_utilities::expect_string_contains_substring(tStream->str(), tSharkAttributeValue, aTestContext);
    plato::test_utilities::expect_string_contains_substring(tStream->str(), tMessage, aTestContext);
    plato::test_utilities::expect_string_contains_substring(tStream->str(), to_string(aSeverity), aTestContext);
}
}  // namespace

TEST(SeverityLogger, LogMembers)
{
    check_log_message(Severity::kDebug, TEST_CONTEXT("Debug log member"));
    check_log_message(Severity::kInfo, TEST_CONTEXT("Info log member"));
    check_log_message(Severity::kWarning, TEST_CONTEXT("Warning log member"));
    check_log_message(Severity::kError, TEST_CONTEXT("Error log member"));
}

}  // namespace plato::third_party_integration::boost_log::unittest
