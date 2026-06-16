#include <gtest/gtest.h>

#include <boost/log/expressions.hpp>  // NOLINT(misc-include-cleaner)
#include <boost/log/expressions/formatters/stream.hpp>
#include <boost/log/expressions/message.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <sstream>
#include <string_view>

#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"

namespace plato::third_party_integration::boost_log::unittest
{
TEST(LoggerSinkSetupTeardown, LogsToStringstream)
{
    const auto tStream = boost::make_shared<std::stringstream>();

    const auto tLogInfoMessage = [tStream](const std::string_view aMessage)
    {
        const auto tFormatter =
            boost::log::formatter{boost::log::expressions::stream << boost::log::expressions::smessage};
        [[maybe_unused]] const auto tInternalLoggerSink =
            LoggerSinkSetupTeardown{tStream, tFormatter, boost::log::filter{}};
        auto tLogger = boost::log::sources::logger{};
        BOOST_LOG(tLogger) << aMessage;
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
