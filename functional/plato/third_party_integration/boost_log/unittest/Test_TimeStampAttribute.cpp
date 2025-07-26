#include <gtest/gtest.h>

#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"
#include "plato/third_party_integration/boost_log/TimeStampAttribute.hpp"

namespace plato::third_party_integration::boost_log::unittest
{
TEST(TimeStampAttribute, Formatter)
{
    const auto tFormatter = time_stamp_formatter();

    const auto tStream = std::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tInternalLoggerSink =
        LoggerSinkSetupTeardown{tStream, tFormatter, boost::log::filter{}};

    auto tLogger = boost::log::sources::logger{};
    tLogger.add_attribute(kTimeStampAttributeName.data(), boost::log::attributes::local_clock());
    BOOST_LOG(tLogger) << "should not appear";

    std::cout << tStream->str();
}
}  // namespace plato::third_party_integration::boost_log::unittest
