#include <gtest/gtest.h>

#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/regex.hpp>

#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"
#include "plato/third_party_integration/boost_log/TimeStampAttribute.hpp"

namespace plato::third_party_integration::boost_log::unittest
{
TEST(TimeStampAttribute, Formatter)
{
    const auto tStream = boost::make_shared<std::stringstream>();

    [[maybe_unused]] const auto tInternalLoggerSink =
        LoggerSinkSetupTeardown{tStream, TimeStampAttribute::formatter(), boost::log::filter{}};

    auto tLogger = boost::log::sources::logger{};
    tLogger.add_attribute(TimeStampAttribute::name().data(), boost::log::attributes::local_clock());
    BOOST_LOG(tLogger) << "should not appear";

    // Check via a regex matching the date/time format
    const auto tDateTimeRegex = "\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\]";
    const auto tRegex = boost::regex{tDateTimeRegex};
    EXPECT_TRUE(boost::regex_search(tStream->str(), tRegex)) << "Result: " << tStream->str();
}
}  // namespace plato::third_party_integration::boost_log::unittest
