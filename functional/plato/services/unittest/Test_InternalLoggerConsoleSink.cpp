#include <gtest/gtest.h>

#include <boost/log/attributes/constant.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include "plato/services/InternalLoggerConsoleSink.hpp"
#include "plato/test_utilities/CoutCerrPrintTestFixture.hpp"
#include "plato/test_utilities/Strings.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"

namespace plato::services::unittest
{
namespace
{
namespace tpi_bl = third_party_integration::boost_log;

class InternalLoggerConsoleSinkCoutRedirect : public plato::test_utilities::CoutCerrPrintTestFixture
{
};

[[nodiscard]] auto test_logger() -> boost::log::sources::logger
{
    auto tLogger = boost::log::sources::logger{};
    tLogger.add_attribute(tpi_bl::LogSourceAttribute<tpi_bl::LogSource::kInternal>::name().data(),
                          boost::log::attributes::constant<tpi_bl::LogSource>(tpi_bl::LogSource::kInternal));
    tLogger.add_attribute(tpi_bl::MPIRankAttribute::name().data(), boost::log::attributes::constant<int>(0));
    return tLogger;
}
}  // namespace

TEST(InternalLoggerConsoleSink, Outputs)
{
    const auto tStream = boost::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLogSink = internal_logger_console_sink(tStream);

    constexpr auto tMessage = std::string_view{"It lives in a string!"};
    auto tLogger = test_logger();
    BOOST_LOG(tLogger) << tMessage;

    test_utilities::expect_string_contains_substring(tStream->str(), tMessage, TEST_CONTEXT("Logger sink output"));
}

TEST_F(InternalLoggerConsoleSinkCoutRedirect, Outputs)
{
    [[maybe_unused]] const auto tLogSink = internal_logger_console_sink();

    constexpr auto tMessage = std::string_view{"It lives in cout!"};
    auto tLogger = test_logger();
    BOOST_LOG(tLogger) << tMessage;

    checkRankZeroCoutStringStream(std::string{tMessage} + "\n", TEST_CONTEXT("Checking for log message."));
}
}  // namespace plato::services::unittest
