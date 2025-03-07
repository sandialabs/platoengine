#include "plato/criteria/library/CriterionRegistration.hpp"

#include <boost/mpi/communicator.hpp>
#include <iterator>
#include <set>

#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_parser/InputDefinitions.hpp"
#include "plato/services/AppConfiguration.hpp"

namespace plato::criteria::library
{
namespace
{
std::string criterion_registration_name(const std::string_view aAppName, const std::string_view aCriterionName)
{
    return std::string{aAppName} + ":" + std::string{aCriterionName};
}

template <std::size_t... kIndices>
[[nodiscard]] auto is_criterion_function_registered_impl(const std::string_view aFunctionName,
                                                         const std::size_t aFactoryIndex,
                                                         const std::index_sequence<kIndices...>) -> bool
{
    return ((aFactoryIndex == kIndices && detail::is_criterion_function_registered<kIndices>(aFunctionName)) || ...);
}
}  // namespace

auto is_criterion_function_registered(const std::string_view aFunctionName, const CriterionTraits aTraits) -> bool
{
    const auto tIndex = detail::factory_index(aTraits.mParallelization, aTraits.mDimension);
    return is_criterion_function_registered_impl(
        aFunctionName, tIndex, std::make_index_sequence<std::tuple_size_v<detail::FactoryRegistrationTypes>>());
}

std::string criterion_registration_name(const services::AppConfiguration& aAppConfiguration,
                                        const services::CriterionConfiguration& aCriterionConfiguration)
{
    return criterion_registration_name(aAppConfiguration.mName, aCriterionConfiguration.mName);
}

std::string criterion_registration_name(const boost::optional<input_parser::AppName>& aAppName,
                                        const input_parser::CriterionName& aCriterionName)
{
    return criterion_registration_name(aAppName ? aAppName->mToken : input_parser::kBuiltinAppName,
                                       aCriterionName.mToken);
}

std::string builtin_criterion_registration_name(const std::string_view aCriterionName)
{
    return criterion_registration_name(input_parser::kBuiltinAppName, aCriterionName);
}

std::set<std::string> registered_criteria_names()
{
    auto tAllCriteria = std::set<std::string>{};
    auto tSerialFunctions = core::registered_function_names<CriterionFunction, CriterionInput>();
    std::move(tSerialFunctions.begin(), tSerialFunctions.end(), std::inserter(tAllCriteria, tAllCriteria.begin()));
    auto tParallelFunctions =
        core::registered_function_names<CriterionFunction, CriterionInput, boost::mpi::communicator>();
    std::move(tParallelFunctions.begin(), tParallelFunctions.end(), std::inserter(tAllCriteria, tAllCriteria.begin()));

    return tAllCriteria;
}

}  // namespace plato::criteria::library
