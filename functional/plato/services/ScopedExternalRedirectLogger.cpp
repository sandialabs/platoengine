#include "plato/services/ScopedExternalRedirectLogger.hpp"

#include "plato/services/ComponentLogger.hpp"
#include "plato/third_party_integration/boost_log/ComponentAttributes.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"
#include "plato/third_party_integration/boost_log/SeverityLogger.hpp"

namespace plato::services
{
template <components::ComponentType kComponentType>
ScopedExternalRedirectLogger<kComponentType>::ScopedExternalRedirectLogger(std::string aComponentName)
    : mComponentName{std::move(aComponentName)},
      mOriginalCoutBuffer{std::cout.rdbuf()},
      mOriginalCerrBuffer{std::cerr.rdbuf()}
{
    std::cout.rdbuf(mRedirectedCoutStream.rdbuf());
    std::cerr.rdbuf(mRedirectedCerrStream.rdbuf());
}

template <components::ComponentType kComponentType>
ScopedExternalRedirectLogger<kComponentType>::~ScopedExternalRedirectLogger()
{
    namespace tpi_bl = third_party_integration::boost_log;
    auto tLogger = tpi_bl::SeverityLogger{tpi_bl::ComponentTypeAndNameAttribute{tpi_bl::ComponentTypeAndName{
                                              .mComponentType = kComponentType, .mComponentName = mComponentName}},
                                          tpi_bl::MPIWorldCommRankAttribute{},
                                          tpi_bl::LogSourceAttribute<tpi_bl::LogSource::kExternal>{},
                                          tpi_bl::ComponentTypeFilterAttribute<kComponentType>{}};

    tLogger.logMessage(mRedirectedCoutStream.str(), tpi_bl::Severity::kInfo);
    tLogger.logMessage(mRedirectedCerrStream.str(), tpi_bl::Severity::kError);

    if (!mRedirectedCerrStream.str().empty())
    {
        // Log cerr to the console as well as an error
        auto tConsoleLogger = ComponentLogger{kComponentType, mComponentName};
        tConsoleLogger.logError(mRedirectedCerrStream.str());
    }
}

template class ScopedExternalRedirectLogger<components::ComponentType::kConstraint>;
template class ScopedExternalRedirectLogger<components::ComponentType::kFilter>;
template class ScopedExternalRedirectLogger<components::ComponentType::kGeometry>;
template class ScopedExternalRedirectLogger<components::ComponentType::kObjective>;
template class ScopedExternalRedirectLogger<components::ComponentType::kProcessManager>;

}  // namespace plato::services
