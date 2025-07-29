#include "plato/services/ComponentLogger.hpp"

#include "plato/third_party_integration/boost_log/ComponentAttributes.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"
#include "plato/third_party_integration/boost_log/SeverityLogger.hpp"

namespace plato::services
{
namespace
{
namespace tpi_bl = third_party_integration::boost_log;
}

struct ComponentLogger::ComponentLoggerImpl
{
    tpi_bl::SeverityLogger mLogger;
};

ComponentLogger::ComponentLogger(components::ComponentType aComponentType, std::string_view aComponentName)
    : mPimpl{std::make_unique<ComponentLogger::ComponentLoggerImpl>(tpi_bl::SeverityLogger{
          tpi_bl::ComponentTypeAndNameAttribute{tpi_bl::ComponentTypeAndName{
              .mComponentType = aComponentType, .mComponentName = std::string{aComponentName}}},
          tpi_bl::MPIWorldCommRankAttribute{}, tpi_bl::LogSourceAttribute<tpi_bl::LogSource::kInternal>{}})}
{
}

void ComponentLogger::logDebugMessage(const std::string_view aMessage)
{
    return mPimpl->mLogger.logMessage(aMessage, tpi_bl::Severity::kDebug);
}

void ComponentLogger::logInfo(const std::string_view aMessage)
{
    return mPimpl->mLogger.logMessage(aMessage, tpi_bl::Severity::kInfo);
}

void ComponentLogger::logWarning(const std::string_view aMessage)
{
    return mPimpl->mLogger.logMessage(aMessage, tpi_bl::Severity::kWarning);
}

void ComponentLogger::logError(const std::string_view aMessage)
{
    return mPimpl->mLogger.logMessage(aMessage, tpi_bl::Severity::kError);
}

ComponentLogger::~ComponentLogger() = default;

}  // namespace plato::services
