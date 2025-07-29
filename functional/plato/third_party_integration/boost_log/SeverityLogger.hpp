#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_SEVERITYLOGGER
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_SEVERITYLOGGER

#include <boost/log/attributes/constant.hpp>
#include <boost/log/sources/logger.hpp>
#include <string_view>

#include "plato/third_party_integration/boost_log/AttributeTypes.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"

namespace plato::third_party_integration::boost_log
{
/// @brief A logger for internal plato messages that are mainly intended for output to the console.
class SeverityLogger
{
   public:
    template <Attribute... Attributes>
    SeverityLogger(const Attributes... aAttributes);

    /// @brief Log a message @a aMessage with severity @a aSeverity.
    void logMessage(std::string_view aMessage, Severity aSeverity);

   private:
    boost::log::sources::logger mLogger;
};

template <Attribute... Attributes>
SeverityLogger::SeverityLogger(const Attributes... aAttributes)
{
    (mLogger.add_attribute(Attributes::name().data(), boost::log::attributes::make_constant(aAttributes.mValue)), ...);
}

}  // namespace plato::third_party_integration::boost_log

#endif
