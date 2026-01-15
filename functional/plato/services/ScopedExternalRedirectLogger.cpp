#include "plato/services/ScopedExternalRedirectLogger.hpp"

#include <optional>

#include "plato/services/SystemLogger.hpp"
#include "plato/third_party_integration/boost_log/ComponentAttributes.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"
#include "plato/third_party_integration/boost_log/SeverityLogger.hpp"

namespace plato::services
{
namespace
{
template <std::size_t kComponentIndex>
void make_severity_logger_impl(const components::ComponentType aComponentType,
                               const std::string& aComponentName,
                               std::optional<third_party_integration::boost_log::SeverityLogger>& aExistingLog)
{
    namespace tpi_bl = third_party_integration::boost_log;
    constexpr auto kComponentType = components::component_type_from_index<kComponentIndex>();
    if (aComponentType == kComponentType)
    {
        aExistingLog = std::optional<tpi_bl::SeverityLogger>{
            std::in_place_t{},
            tpi_bl::ComponentTypeAndNameAttribute{
                tpi_bl::ComponentTypeAndName{.mComponentType = aComponentType, .mComponentName = aComponentName}},
            tpi_bl::MPIRankAttribute{}, tpi_bl::LogSourceAttribute<tpi_bl::LogSource::kExternal>{},
            tpi_bl::ComponentTypeFilterAttribute<kComponentType>{}};
    }
}

template <std::size_t... kComponentIndices>
[[nodiscard]] auto make_severity_logger_impl(const components::ComponentType aComponentType,
                                             const std::string& aComponentName,
                                             std::integer_sequence<std::size_t, kComponentIndices...>)
{
    auto tLogger = std::optional<third_party_integration::boost_log::SeverityLogger>{};
    ((make_severity_logger_impl<kComponentIndices>(aComponentType, aComponentName, tLogger)), ...);
    assert(tLogger.has_value());
    return tLogger.value();
}

[[nodiscard]] auto make_severity_logger(const components::ComponentType aComponentType,
                                        const std::string& aComponentName)
    -> third_party_integration::boost_log::SeverityLogger
{
    return make_severity_logger_impl(
        aComponentType, aComponentName,
        std::make_index_sequence<utilities::number_of_enumerates<components::ComponentType>()>());
}

}  // namespace

ScopedExternalRedirectLogger::ScopedExternalRedirectLogger(const components::ComponentType aComponentType,
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

    auto tLogger = make_severity_logger(mComponentType, mComponentName);

    if (!mRedirectedCoutStream.str().empty())
    {
        // Add a spacer
        tLogger.logMessage("\n" + mRedirectedCoutStream.str(), tpi_bl::Severity::kInfo);
    }
    if (!mRedirectedCerrStream.str().empty())
    {
        tLogger.logMessage(mRedirectedCerrStream.str(), tpi_bl::Severity::kError);
        // Log an error to the console as well
        auto tConsoleLogger = component_logger(mComponentType, mComponentName);
        tConsoleLogger.logError("\n" + mRedirectedCerrStream.str());
    }

    std::cout.rdbuf(mOriginalCoutBuffer);
    std::cerr.rdbuf(mOriginalCerrBuffer);
}

}  // namespace plato::services
