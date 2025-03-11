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
constexpr auto kFactoryRegistrationTypesSequence =
    std::make_index_sequence<std::tuple_size_v<FactoryRegistrationTypes>>();

template <std::size_t kIndex>
constexpr auto kFactorySignatureSequence =
    std::make_index_sequence<std::tuple_size_v<std::tuple_element_t<kIndex, FactoryRegistrationTypes>>>();

std::string criterion_registration_name(const std::string_view aAppName, const std::string_view aCriterionName)
{
    return std::string{aAppName} + ":" + std::string{aCriterionName};
}

template <typename Tuple, std::size_t... Indices>
constexpr auto is_criterion_function_registered_in_specific_factory_impl(const std::string_view aFunctionName,
                                                                         const std::index_sequence<Indices...>)
{
    return core::is_factory_function_registered<std::tuple_element_t<Indices, Tuple>...>(aFunctionName);
}

template <std::size_t kFactoryIndex>
constexpr auto is_criterion_function_registered_in_specific_factory(const std::string_view aFunctionName)
{
    using FactoryTypes = std::tuple_element_t<kFactoryIndex, FactoryRegistrationTypes>;
    return is_criterion_function_registered_in_specific_factory_impl<FactoryTypes>(
        aFunctionName, kFactorySignatureSequence<kFactoryIndex>);
}

template <std::size_t... kIndices>
[[nodiscard]] auto is_criterion_function_registered_in_any_factory(const std::string_view aFunctionName,
                                                                   const std::size_t aFactoryIndex,
                                                                   const std::index_sequence<kIndices...>) -> bool
{
    return (
        (aFactoryIndex == kIndices && is_criterion_function_registered_in_specific_factory<kIndices>(aFunctionName)) ||
        ...);
}

template <typename FactorySignatureTuple, std::size_t... kIndices>
void registered_criteria_names_from_single_factory(std::set<std::string>& aAllCriteria,
                                                   const std::index_sequence<kIndices...>)
{
    auto tFunctionNames = core::registered_function_names<std::tuple_element_t<kIndices, FactorySignatureTuple>...>();
    std::move(tFunctionNames.begin(), tFunctionNames.end(), std::inserter(aAllCriteria, aAllCriteria.begin()));
}

template <std::size_t... kIndices>
[[nodiscard]] auto registered_criteria_names_impl(const std::index_sequence<kIndices...>) -> std::set<std::string>
{
    auto tAllCriteria = std::set<std::string>{};
    (registered_criteria_names_from_single_factory<std::tuple_element_t<kIndices, FactoryRegistrationTypes>>(
         tAllCriteria, kFactorySignatureSequence<kIndices>),
     ...);
    return tAllCriteria;
}
}  // namespace

auto is_criterion_function_registered(const std::string_view aFunctionName, const CriterionTraits aTraits) -> bool
{
    const auto tIndex = trait_index(aTraits.mParallelization, aTraits.mDimension);
    return is_criterion_function_registered_in_any_factory(aFunctionName, tIndex, kFactoryRegistrationTypesSequence);
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
    return registered_criteria_names_impl(kFactoryRegistrationTypesSequence);
}

}  // namespace plato::criteria::library
