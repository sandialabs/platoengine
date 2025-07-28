#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_COMPONENTLOGGER
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_COMPONENTLOGGER

#include <boost/log/sources/logger.hpp>
#include <string_view>

#include "plato/components/ComponentType.hpp"

namespace plato::third_party_integration::boost_log
{
/// @brief A logger for internal plato messages that are mainly intended for output to the console.
class ComponentLogger
{
   public:
    ComponentLogger(components::ComponentType aComponentType, std::string_view aComponentName);

    /// @brief Log a debug message.
    ///
    /// Debug messages are meant to be used for development and should not be left in production code.
    void logDebugMessage(std::string_view aMessage);

    /// @brief Log an informational message.
    ///
    /// Informational messages are meant to be sparingly used to not clutter the output, and to not communicate warnings
    /// or errors.
    void logInfo(std::string_view aMessage);

    /// @brief Log a warning message.
    ///
    /// Warning messages are meant to communicate an issue with the code that should be addressed, but execution can
    /// continue.
    void logWarning(std::string_view aMessage);

    /// @brief Log an error message.
    ///
    /// Error messages are meant to communicate an issue with the code that caused the code to halt.
    void logError(std::string_view aMessage);

   private:
    boost::log::sources::logger mLogger;
};

}  // namespace plato::third_party_integration::boost_log

#endif
