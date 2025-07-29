#include "plato/services/ScopedExternalRedirectLogger.hpp"

#include "plato/third_party_integration/boost_log/ComponentAttributes.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"
#include "plato/third_party_integration/boost_log/SeverityLogger.hpp"

namespace plato::services
{
ScopedExternalRedirectLogger::ScopedExternalRedirectLogger(components::ComponentType aComponentType,
                                                           std::string aComponentName)
    : mComponentType{aComponentType},
      mComponentName{std::move(aComponentName)},
      mOriginalCoutBuffer{std::cout.rdbuf()},
      mOriginalCerrBuffer{std::cerr.rdbuf()}
{
    std::cout.rdbuf(mRedirectedCoutStream.rdbuf());
    std::cerr.rdbuf(mRedirectedCerrStream.rdbuf());
}

ScopedExternalRedirectLogger::~ScopedExternalRedirectLogger()
{
    namespace tpi_bl = third_party_integration::boost_log;
    auto tLogger = tpi_bl::SeverityLogger{tpi_bl::ComponentTypeAndNameAttribute{tpi_bl::ComponentTypeAndName{
                                              .mComponentType = mComponentType, .mComponentName = mComponentName}},
                                          tpi_bl::MPIWorldCommRankAttribute{},
                                          tpi_bl::LogSourceAttribute<tpi_bl::LogSource::kExternal>{}};

    tLogger.logMessage(mRedirectedCoutStream.str(), tpi_bl::Severity::kInfo);
    tLogger.logMessage(mRedirectedCerrStream.str(), tpi_bl::Severity::kError);
}

}  // namespace plato::services
