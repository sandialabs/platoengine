#include <gtest/gtest.h>

#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <sstream>

#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/LogSourceLoggerSinkSetupTeardown.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"

namespace plato::third_party_integration::boost_log::unittest
{
TEST(InternalLoggerSinkSetupTeardown, LogsToStringstream)
{
    const auto tStream = std::make_shared<std::stringstream>();

    const auto tLogInfoMessage = [tStream](const std::string_view aMessage, const LogSource aLogSource)
    {
        [[maybe_unused]] const auto tInternalLoggerSink = InternalLoggerSinkSetupTeardown{tStream};
        auto tLogger = boost::log::sources::severity_logger<Severity>{};
        BOOST_LOG_SCOPED_THREAD_TAG(kLogSourceAttributeName.data(), aLogSource);
        BOOST_LOG_SEV(tLogger, Severity::kInfo) << aMessage;
    };

    constexpr auto tFirstMessage = std::string_view{"Message the first."};
    tLogInfoMessage(tFirstMessage, LogSource::kInternal);
    const auto tExpected = std::string{tFirstMessage} + "\n";
    EXPECT_EQ(tStream->str(), tExpected);

    tLogInfoMessage("An unlogged external message.",
                    LogSource::kExternal);  // Use kExternal, nothing should be added to the string
    EXPECT_EQ(tStream->str(), tExpected);
}

}  // namespace plato::third_party_integration::boost_log::unittest
