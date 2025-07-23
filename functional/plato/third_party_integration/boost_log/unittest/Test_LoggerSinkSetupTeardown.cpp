#include <gtest/gtest.h>

#include <boost/log/expressions.hpp>
#include <boost/log/expressions/formatters/stream.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <sstream>

#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"

namespace plato::third_party_integration::boost_log::unittest
{
TEST(LoggerSinkSetupTeardown, LogsToStringstream)
{
    const auto tStream = std::make_shared<std::stringstream>();

    const auto tLogInfoMessage = [tStream](const std::string_view aMessage)
    {
        const auto tFormatter =
            boost::log::formatter{boost::log::expressions::stream << boost::log::expressions::smessage};
        [[maybe_unused]] const auto tInternalLoggerSink =
            LoggerSinkSetupTeardown{tStream, tFormatter, boost::log::filter{}};
        auto tLogger = boost::log::sources::severity_logger<Severity>{};
        BOOST_LOG_SEV(tLogger, Severity::kInfo) << aMessage;
    };

    constexpr auto tFirstMessage = std::string_view{"Message the first."};
    {
        tLogInfoMessage(tFirstMessage);
        const auto tExpected = std::string{tFirstMessage} + "\n";
        EXPECT_EQ(tStream->str(), tExpected);
    }
    constexpr auto tSecondMessage = std::string_view{"Message the second."};
    {
        tLogInfoMessage(tSecondMessage);
        const auto tExpected = std::string{tFirstMessage} + "\n" + std::string{tSecondMessage} + "\n";
        EXPECT_EQ(tStream->str(), tExpected);
    }
}

}  // namespace plato::third_party_integration::boost_log::unittest
