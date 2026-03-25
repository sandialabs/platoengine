#include "plato/filter/library/FilterLogger.hpp"

namespace plato::filter::library
{
auto filter_field_task_log(std::string_view aFilterName) -> services::TaskLogSetupTeardown
{
    return services::TaskLogSetupTeardown{"Filtering field", detail::filter_logger(aFilterName)};
}

auto jacobian_task_log(std::string_view aFilterName) -> services::TaskLogSetupTeardown
{
    return services::TaskLogSetupTeardown{services::jacobian_task_message(), detail::filter_logger(aFilterName)};
}

auto adjoint_jacobian_task_log(std::string_view aFilterName) -> services::TaskLogSetupTeardown
{
    return services::TaskLogSetupTeardown{services::adjoint_jacobian_task_message(),
                                          detail::filter_logger(aFilterName)};
}

namespace detail
{
[[nodiscard]] auto filter_logger(const std::string_view aFilterName) -> services::SystemLogger
{
    return services::component_logger(components::ComponentType::kFilter, aFilterName);
}
}  // namespace detail
}  // namespace plato::filter::library
