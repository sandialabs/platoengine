#include "plato/services/ExternalLoggerFileSink.hpp"

#include <boost/mpi/communicator.hpp>
#include <boost/shared_ptr.hpp>
#include <fstream>
#include <string>

#include "plato/components/ComponentTypeStream.hpp"
#include "plato/services/ExternalLoggerFileSinkDetail.hpp"

namespace plato::services
{
namespace
{
template <std::size_t kComponentIndex>
[[nodiscard]] auto component_external_log_sink() -> third_party_integration::boost_log::LoggerSinkSetupTeardown
{
    constexpr auto tComponentType = components::component_type_from_index<kComponentIndex>();
    return detail::external_logger_file_sink<tComponentType>(
        external_log_file_path(tComponentType, boost::mpi::communicator{}));
};
}  // namespace

auto external_log_file_path(const components::ComponentType aComponentType,
                            const boost::mpi::communicator& aCommunicator) -> std::filesystem::path
{
    auto tComponentTypeAsString = components::to_string(aComponentType);
    std::replace(tComponentTypeAsString.begin(), tComponentTypeAsString.end(), ' ', '-');
    const auto tBasePath = std::filesystem::path{"logs"};
    return tBasePath /
           std::filesystem::path{tComponentTypeAsString + "-rank-" + std::to_string(aCommunicator.rank()) + ".txt"};
}

[[nodiscard]] auto component_external_logger_file_sinks() -> ComponentExternalLoggerSinks
{
    const auto tLogSinks =
        []<std::size_t... kComponentIndices>(const std::integer_sequence<std::size_t, kComponentIndices...>)
    { return std::array{component_external_log_sink<kComponentIndices>()...}; };

    constexpr auto tNumberOfComponents = utilities::number_of_enumerates<components::ComponentType>();
    return tLogSinks(std::make_index_sequence<tNumberOfComponents>());
}

}  // namespace plato::services
