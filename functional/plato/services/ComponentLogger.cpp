#include "plato/services/ComponentLogger.hpp"

#include "plato/third_party_integration/boost_log/ComponentAttributes.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"
#include "plato/third_party_integration/boost_log/SeverityLogger.hpp"

namespace plato::services
{
struct ComponentLogger::ComponentLoggerImpl
{
    third_party_integration::boost_log::SeverityLogger mLogger;
};

ComponentLogger::ComponentLogger(components::ComponentType aComponentType, std::string_view aComponentName)
    : mPimpl{std::make_unique<ComponentLogger::ComponentLoggerImpl>(third_party_integration::boost_log::SeverityLogger{
          third_party_integration::boost_log::ComponentTypeAndNameAttribute{
              third_party_integration::boost_log::ComponentTypeAndName{.mComponentType = aComponentType,
                                                                       .mComponentName = std::string{aComponentName}}},
          third_party_integration::boost_log::MPIWorldCommRankAttribute{},
          third_party_integration::boost_log::LogSourceAttribute<
              third_party_integration::boost_log::LogSource::kInternal>{}

      })}
{
}

void ComponentLogger::logDebugMessage(const std::string_view aMessage)
{
    return mPimpl->mLogger.logMessage(aMessage, third_party_integration::boost_log::Severity::kDebug);
}

void ComponentLogger::logInfo(const std::string_view aMessage)
{
    return mPimpl->mLogger.logMessage(aMessage, third_party_integration::boost_log::Severity::kInfo);
}

void ComponentLogger::logWarning(const std::string_view aMessage)
{
    return mPimpl->mLogger.logMessage(aMessage, third_party_integration::boost_log::Severity::kWarning);
}

void ComponentLogger::logError(const std::string_view aMessage)
{
    return mPimpl->mLogger.logMessage(aMessage, third_party_integration::boost_log::Severity::kError);
}

ComponentLogger::~ComponentLogger() = default;

}  // namespace plato::services
