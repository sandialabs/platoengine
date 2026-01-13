#ifndef PLATO_CRITERIA_LIBRARY_LOGGINGFUNCTION
#define PLATO_CRITERIA_LIBRARY_LOGGINGFUNCTION

#include <format>
#include <string>

#include "plato/components/ComponentType.hpp"
#include "plato/core/Function.hpp"
#include "plato/services/SystemLogger.hpp"
#include "plato/services/TaskLogSetupTeardown.hpp"

namespace plato::linear_algebra
{
template <typename T>
class DynamicVector;
}

namespace plato::criteria::library
{
/// @brief Wraps @a aFunction with logging.
///
/// The function value will be output directly and the gradients will only be reported as being computed.
template <typename Domain, typename... Info>
[[nodiscard]] auto make_logging_function(const core::Function<Domain, Info...>& aFunction,
                                         components::ComponentType aComponentType,
                                         std::string_view aName);

namespace detail
{
template <typename T>
constexpr inline auto kFormatSpecification = std::string_view{"{}"};

template <>
constexpr inline auto kFormatSpecification<double> = std::string_view{"{:9.8e}"};

template <>
constexpr inline auto kFormatSpecification<linear_algebra::DynamicVector<double>> = std::string_view{"{:10:9.8e}"};
}  // namespace detail

template <typename Domain, typename... Info>
[[nodiscard]] auto make_logging_function(const core::Function<Domain, Info...>& aFunction,
                                         components::ComponentType aComponentType,
                                         std::string_view aName)
{
    return core::Function<Domain, Info...>{
        [mLocalFunction = aFunction, aComponentType, mName = std::string{aName}](Domain aArgument)
        {
            if constexpr (Info::order == core::evaluation::kFunction)
            {
                [[maybe_unused]] auto tLogger = services::component_logger(aComponentType, mName);
                tLogger.logInfo("Evaluating criterion");
                auto tResult = mLocalFunction.template evaluate<Info::order, Info::ordering>(aArgument);
                const auto tLogMessage = std::string{"Evaluation complete. Criterion value = "} +
                                         std::string{detail::kFormatSpecification<typename Info::Codomain>};
                tLogger.logInfo(std::vformat(tLogMessage, std::make_format_args(tResult)));
                return tResult;
            }
            else if (Info::order == core::evaluation::kFirstDerivative)
            {
                [[maybe_unused]] const auto tTaskLogger =
                    services::TaskLogSetupTeardown{"Gradient", services::component_logger(aComponentType, mName)};
                return mLocalFunction.template evaluate<Info::order, Info::ordering>(aArgument);
            }
            return mLocalFunction.template evaluate<Info::order, Info::ordering>(aArgument);
        }...};
}
}  // namespace plato::criteria::library

#endif
