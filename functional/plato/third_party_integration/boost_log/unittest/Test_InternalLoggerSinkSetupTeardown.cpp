#include <gtest/gtest.h>

#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <sstream>

#include "plato/third_party_integration/boost_log/InternalLoggerSinkSetupTeardown.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"

namespace plato::third_party_integration::boost_log::unittest
{
TEST(InternalLoggerSinkSetupTeardown, LogsToStringstream)
{
    initialize_internal_console_sink();

    const auto tStream = std::make_shared<std::stringstream>();

    const auto tLogInfoMessage = [tStream](const std::string_view aMessage)
    {
        [[maybe_unused]] const auto tInternalLoggerSink = InternalLoggerSinkSetupTeardown{tStream};
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
