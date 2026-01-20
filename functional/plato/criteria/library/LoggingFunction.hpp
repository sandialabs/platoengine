#ifndef PLATO_CRITERIA_LIBRARY_LOGGINGFUNCTION
#define PLATO_CRITERIA_LIBRARY_LOGGINGFUNCTION

#include <boost/mpi/communicator.hpp>
#include <format>
#include <string>

#include "plato/components/ComponentType.hpp"
#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/services/SystemLogger.hpp"
#include "plato/services/TaskLogSetupTeardown.hpp"

namespace plato::criteria::library
{
/// @brief Wraps @a aFunction with logging.
///
/// The function value will be output directly and the gradients will only be reported as being computed.
template <typename Domain, typename... Info>
[[nodiscard]] auto make_logging_function(const core::Function<Domain, Info...>& aFunction,
                                         components::ComponentType aComponentType,
                                         std::string_view aName,
                                         const boost::mpi::communicator& aCommunicator = {});

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
                                         std::string_view aName,
                                         const boost::mpi::communicator& aCommunicator)
{
    return core::Function<Domain, Info...>{
        [mLocalFunction = aFunction, aComponentType, mName = std::string{aName}, aCommunicator](Domain aArgument)
        {
            if constexpr (Info::order == core::evaluation::kFunction)
            {
                [[maybe_unused]] auto tLogger = services::component_logger(aComponentType, mName, aCommunicator);
                tLogger.logInfo("Criterion evaluation");
                auto tResult = mLocalFunction.template evaluate<Info::order, Info::ordering>(aArgument);
                const auto tLogMessage = std::string{"Criterion evaluation complete. Result: "} +
                                         std::string{detail::kFormatSpecification<typename Info::Codomain>};
                tLogger.logInfo(std::vformat(tLogMessage, std::make_format_args(tResult)));
                return tResult;
            }
            else if constexpr (Info::order == core::evaluation::kFirstDerivative &&
                               std::same_as<std::remove_cvref_t<typename Info::Codomain>,
                                            std::remove_cvref_t<linear_algebra::DynamicVector<double>>>)
            {
                auto tLogger = services::component_logger(aComponentType, mName, aCommunicator);
                tLogger.logInfo("Gradient evaluation");
                auto tGradient = mLocalFunction.template evaluate<Info::order, Info::ordering>(aArgument);
                const auto tLogMessage = std::string{"Gradient evaluation complete. Result norm: "} +
                                         std::string{detail::kFormatSpecification<double>};
                const auto tGradientNorm = linear_algebra::norm(tGradient);
                tLogger.logInfo(std::vformat(tLogMessage, std::make_format_args(tGradientNorm)));
                return tGradient;
            }
            else if constexpr (Info::order == core::evaluation::kFirstDerivative)
            {
                [[maybe_unused]] const auto tTaskLogger = services::TaskLogSetupTeardown{
                    "Gradient evaluation", services::component_logger(aComponentType, mName, aCommunicator)};
                return mLocalFunction.template evaluate<Info::order, Info::ordering>(aArgument);
            }
            else
            {
                return mLocalFunction.template evaluate<Info::order, Info::ordering>(aArgument);
            }
        }...};
}
}  // namespace plato::criteria::library

#endif
