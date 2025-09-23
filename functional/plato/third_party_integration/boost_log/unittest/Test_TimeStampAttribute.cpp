#include <gtest/gtest.h>

#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <regex>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"
#include "plato/third_party_integration/boost_log/TimeStampAttribute.hpp"
#include "plato/utilities/Colorize.hpp"

namespace plato::third_party_integration::boost_log::unittest
{
namespace
{
constexpr auto kDateTimeRegex = std::string_view{"[.*]*[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}[.*]*"};

void check_time_regex(const std::string_view aTimeStampRegex,
                      const std::stringstream& aLogStream,
                      const plato::test_utilities::TestContext& aTestContext)
{
    auto tLogger = boost::log::sources::logger{};
    tLogger.add_attribute(TimeStampAttribute::name().data(), boost::log::attributes::local_clock());
    BOOST_LOG(tLogger) << "should not appear";

    // Check via a regex matching the date/time format
    const auto tRegex = std::regex{aTimeStampRegex.data()};
    EXPECT_TRUE(std::regex_search(aLogStream.str(), tRegex))
        << aTestContext << "Result: " << aLogStream.str() << "\nRegex: " << aTimeStampRegex;
}
}  // namespace

TEST(TimeStampAttribute, FormatterNoStyle)
{
    const auto tLogStream = boost::make_shared<std::stringstream>();

    [[maybe_unused]] const auto tInternalLoggerSink = LoggerSinkSetupTeardown{
        tLogStream, TimeStampAttribute::formatter(FormattingStyle::kNone), boost::log::filter{}};

    check_time_regex(kDateTimeRegex, *tLogStream, TEST_CONTEXT("No style in formatter"));
}

TEST(TimeStampAttribute, FormatterColor)
{
    const auto tLogStream = boost::make_shared<std::stringstream>();

    [[maybe_unused]] const auto tInternalLoggerSink = LoggerSinkSetupTeardown{
        tLogStream, TimeStampAttribute::formatter(FormattingStyle::kColor), boost::log::filter{}};

    check_time_regex(kDateTimeRegex, *tLogStream, TEST_CONTEXT("Color style"));
}
}  // namespace plato::third_party_integration::boost_log::unittest
