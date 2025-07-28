#include <gtest/gtest.h>

#include <boost/log/attributes/constant.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include "plato/test_utilities/CoutPrintTestFixture.hpp"
#include "plato/third_party_integration/boost_log/InternalLoggerConsoleSink.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"

namespace plato::third_party_integration::boost_log::unittest
{
namespace
{
class InternalLoggerConsoleSinkCoutRedirect : public plato::test_utilities::CoutPrintTestFixture
{
};

[[nodiscard]] auto test_logger() -> boost::log::sources::logger
{
    auto tLogger = boost::log::sources::logger{};
    tLogger.add_attribute(LogSourceAttribute<LogSource::kInternal>::name().data(),
                          boost::log::attributes::constant<LogSource>(LogSource::kInternal));
    tLogger.add_attribute(MPIWorldCommRankAttribute::name().data(), boost::log::attributes::constant<int>(0));
    return tLogger;
}
}  // namespace

TEST(InternalLoggerConsoleSink, Outputs)
{
    const auto tStream = std::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLogSink = internal_console_sink(tStream);

    constexpr auto tMessage = std::string_view{"It lives in a string!"};
    auto tLogger = test_logger();
    BOOST_LOG(tLogger) << tMessage;

    EXPECT_NE(tStream->str().find(tMessage), std::string::npos) << "Result: " << tStream->str();
}

TEST_F(InternalLoggerConsoleSinkCoutRedirect, Outputs)
{
    [[maybe_unused]] const auto tLogSink = internal_console_sink();

    constexpr auto tMessage = std::string_view{"It lives in cout!"};
    auto tLogger = test_logger();
    BOOST_LOG(tLogger) << tMessage;

    checkRankZeroStringStreamForPattern({std::string{tMessage}}, TEST_CONTEXT("Checking for log message."));
}
}  // namespace plato::third_party_integration::boost_log::unittest
