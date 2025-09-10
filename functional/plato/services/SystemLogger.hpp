#ifndef PLATO_SERVICES_SYSTEMLOGGER
#define PLATO_SERVICES_SYSTEMLOGGER

#include <any>
#include <string_view>

#include "plato/components/ComponentType.hpp"

namespace plato::services
{
/// @brief A logger for internal plato messages that will be output to the console.
class SystemLogger
{
   public:
    /// @brief Default ctor should be used for logs that are not component-specific.
    /// @sa system_logger
    SystemLogger();

    /// @brief Creates a logger that will log the component type and name as an attribute along with the log message.
    /// Prefer this logger, which provides more context to messages.
    /// @sa component_logger
    SystemLogger(components::ComponentType aComponentType, std::string_view aComponentName);

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
    std::any mImpl;
};

/// @brief Helper function for constructing a SystemLogger with no component context.
[[nodiscard]] auto system_logger() -> SystemLogger;

/// @brief Helper function for constructing a SystemLogger with component context.
[[nodiscard]] auto component_logger(components::ComponentType aComponentType, std::string_view aComponentName)
    -> SystemLogger;

}  // namespace plato::services

#endif
