#include <gtest/gtest.h>

#include <boost/log/attributes/constant.hpp>
#include <boost/log/expressions/formatters/stream.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <sstream>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"
#include "plato/utilities/Colorize.hpp"
#include "plato/utilities/StringUtilities.hpp"

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

TEST(Severity, Formatter)
{
    const auto tCheckSeverityOutput = [](const FormattingStyle aFormattingStyle, const std::string_view aExpected,
                                         const plato::test_utilities::TestContext& aTestContext)
    {
        const auto tStream = boost::make_shared<std::stringstream>();
        const auto tFormatter = SeverityAttribute::formatter(aFormattingStyle);
        [[maybe_unused]] const auto tInternalLoggerSink =
            LoggerSinkSetupTeardown{tStream, tFormatter, boost::log::filter{}};

        auto tLogger = boost::log::sources::logger{};
        tLogger.add_attribute(SeverityAttribute::name().data(),
                              boost::log::attributes::make_constant(Severity::kError));

        BOOST_LOG(tLogger) << "this message should not appear";

        EXPECT_EQ(tStream->str(), aExpected) << aTestContext;
    };

    constexpr auto tExpectedNoColorOutput = std::string_view{"[error] \n"};
    tCheckSeverityOutput(FormattingStyle::kNone, tExpectedNoColorOutput, TEST_CONTEXT("No color"));

    const auto tExpectedColorOutput =
        utilities::concatenate("[", utilities::color_code(utilities::TextColor::kRed), "error",
                               utilities::color_code(utilities::TextColor::kDefault), "] \n");
    tCheckSeverityOutput(FormattingStyle::kColor, tExpectedColorOutput, TEST_CONTEXT("Color"));
}

}  // namespace plato::third_party_integration::boost_log::unittest
