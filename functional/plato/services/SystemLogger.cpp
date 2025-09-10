#include "plato/services/SystemLogger.hpp"

#include "plato/third_party_integration/boost_log/ComponentAttributes.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"
#include "plato/third_party_integration/boost_log/SeverityLogger.hpp"

namespace plato::services
{
namespace
{
namespace tpi_bl = third_party_integration::boost_log;

struct SystemLoggerImpl
{
    tpi_bl::SeverityLogger mLogger;
};

[[nodiscard]] auto make_system_logger_impl(tpi_bl::SeverityLogger&& aSeverityLogger) -> std::any
{
    return std::make_any<SystemLoggerImpl>(SystemLoggerImpl{std::move(aSeverityLogger)});
}

[[nodiscard]] auto system_logger(std::any& aImpl) -> tpi_bl::SeverityLogger&
{
    return std::any_cast<SystemLoggerImpl>(&aImpl)->mLogger;
}
}  // namespace

SystemLogger::SystemLogger()
    : mImpl{make_system_logger_impl(tpi_bl::SeverityLogger{tpi_bl::MPIWorldCommRankAttribute{},
                                                           tpi_bl::LogSourceAttribute<tpi_bl::LogSource::kInternal>{}})}
{
}

SystemLogger::SystemLogger(components::ComponentType aComponentType, std::string_view aComponentName)
    : mImpl{make_system_logger_impl(tpi_bl::SeverityLogger{
          tpi_bl::ComponentTypeAndNameAttribute{tpi_bl::ComponentTypeAndName{
              .mComponentType = aComponentType, .mComponentName = std::string{aComponentName}}},
          tpi_bl::MPIWorldCommRankAttribute{}, tpi_bl::LogSourceAttribute<tpi_bl::LogSource::kInternal>{}})}
{
}

void SystemLogger::logDebugMessage(const std::string_view aMessage)
{
    return system_logger(mImpl).logMessage(aMessage, tpi_bl::Severity::kDebug);
}

void SystemLogger::logInfo(const std::string_view aMessage)
{
    return system_logger(mImpl).logMessage(aMessage, tpi_bl::Severity::kInfo);
}

void SystemLogger::logWarning(const std::string_view aMessage)
{
    return system_logger(mImpl).logMessage(aMessage, tpi_bl::Severity::kWarning);
}

void SystemLogger::logError(const std::string_view aMessage)
{
    return system_logger(mImpl).logMessage(aMessage, tpi_bl::Severity::kError);
}

auto system_logger() -> SystemLogger { return SystemLogger{}; }

auto component_logger(components::ComponentType aComponentType, std::string_view aComponentName) -> SystemLogger
{
    return SystemLogger{aComponentType, aComponentName};
}

}  // namespace plato::services
