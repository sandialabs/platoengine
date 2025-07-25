#include <gtest/gtest.h>

#include <boost/log/attributes/constant.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include "plato/third_party_integration/boost_log/InternalLoggerConsoleSink.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"

namespace plato::third_party_integration::boost_log::unittest
{
TEST(InternalLoggerConsoleSink, Outputs)
{
    // TODO capture the output for a real test
    [[maybe_unused]] const auto tLogSink = internal_console_sink();

    auto tLogger = boost::log::sources::logger{};
    tLogger.add_attribute(kLogSourceAttributeName.data(),
                          boost::log::attributes::constant<LogSource>(LogSource::kInternal));
    tLogger.add_attribute(kMPIRankAttributeName.data(), boost::log::attributes::constant<int>(0));

    BOOST_LOG(tLogger) << "It lives!";
}
}  // namespace plato::third_party_integration::boost_log::unittest
