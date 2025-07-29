#ifndef PLATO_SERVICES_COMPONENTLOGGER
#define PLATO_SERVICES_COMPONENTLOGGER

#include <memory>
#include <string_view>

#include "plato/components/ComponentType.hpp"

namespace plato::services
{
/// @brief A logger for internal plato messages from system components (such as filters) that are intended for
/// output to the console.
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

    ~ComponentLogger();
    ComponentLogger(const ComponentLogger&) = delete;
    auto operator=(const ComponentLogger&) -> ComponentLogger& = delete;
    ComponentLogger(ComponentLogger&&) = default;
    auto operator=(ComponentLogger&&) -> ComponentLogger& = default;

   private:
    struct ComponentLoggerImpl;
    std::unique_ptr<ComponentLoggerImpl> mPimpl;
};

}  // namespace plato::services

#endif
