#include "plato/criteria/library/CriterionRegistration.hpp"

#include <algorithm>
#include <boost/optional/optional.hpp>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

#include "plato/criteria/library/CriterionTraits.hpp"
#include "plato/input_parser/InputDefinitions.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
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

auto criterion_registration_name(const std::string_view aAppName, const std::string_view aCriterionName) -> std::string
{
    return std::string{aAppName} + ":" + std::string{aCriterionName};
}

template <typename Tuple, std::size_t... kIndices>
constexpr auto is_criterion_function_registered_in_specific_factory_impl(
    const std::string_view aFunctionName, const std::index_sequence<kIndices...> /*unused*/)
{
    return core::is_factory_function_registered<std::tuple_element_t<kIndices, Tuple>...>(aFunctionName);
}

template <std::size_t kFactoryIndex>
constexpr auto is_criterion_function_registered_in_specific_factory(const std::string_view aFunctionName)
{
    using FactoryTypes = std::tuple_element_t<kFactoryIndex, FactoryRegistrationTypes>;
    return is_criterion_function_registered_in_specific_factory_impl<FactoryTypes>(
        aFunctionName, kFactorySignatureSequence<kFactoryIndex>);
}

template <std::size_t... kIndices>
[[nodiscard]] auto is_criterion_function_registered_in_factory_with_index(
    const std::string_view aFunctionName,
    const std::size_t aFactoryIndex,
    const std::index_sequence<kIndices...> /*unused*/) -> bool
{
    return (
        (aFactoryIndex == kIndices && is_criterion_function_registered_in_specific_factory<kIndices>(aFunctionName)) ||
        ...);
}

template <std::size_t... kIndices>
[[nodiscard]] auto is_criterion_function_registered_in_any_factory(
    const std::string_view aFunctionName, const std::index_sequence<kIndices...> /*unused*/) -> bool
{
    return (is_criterion_function_registered_in_specific_factory<kIndices>(aFunctionName) || ...);
}

template <typename FactorySignatureTuple, std::size_t... kIndices>
void registered_criteria_names_from_single_factory(std::set<std::string>& aAllCriteria,
                                                   const std::index_sequence<kIndices...> /*unused*/)
{
    auto tFunctionNames = core::registered_function_names<std::tuple_element_t<kIndices, FactorySignatureTuple>...>();
    std::move(tFunctionNames.begin(), tFunctionNames.end(), std::inserter(aAllCriteria, aAllCriteria.begin()));
}

template <std::size_t... kIndices>
[[nodiscard]] auto registered_criteria_names_impl(const std::index_sequence<kIndices...> /*unused*/)
    -> std::set<std::string>
{
    auto tAllCriteria = std::set<std::string>{};
    (registered_criteria_names_from_single_factory<std::tuple_element_t<kIndices, FactoryRegistrationTypes>>(
         tAllCriteria, kFactorySignatureSequence<kIndices>),
     ...);
    return tAllCriteria;
}
}  // namespace

auto is_criterion_function_registered(const std::string_view aFunctionName) -> bool
{
    return is_criterion_function_registered_in_any_factory(aFunctionName, kFactoryRegistrationTypesSequence);
}

auto criterion_function_has_traits(const std::string_view aFunctionName, const CriterionTraits aTraits) -> bool
{
    const auto tIndex = trait_index(aTraits.mParallelization, aTraits.mDimension);
    return is_criterion_function_registered_in_factory_with_index(aFunctionName, tIndex,
                                                                  kFactoryRegistrationTypesSequence);
}

auto criterion_registration_name(const services::AppConfiguration& aAppConfiguration,
                                 const services::CriterionConfiguration& aCriterionConfiguration) -> std::string
{
    return criterion_registration_name(aAppConfiguration.mName, aCriterionConfiguration.mName);
}

auto criterion_registration_name(const boost::optional<input_parser::AppName>& aAppName,
                                 const input_parser::CriterionName& aCriterionName) -> std::string
{
    return criterion_registration_name(aAppName ? aAppName->mToken : input_parser::kBuiltinAppName,
                                       aCriterionName.mToken);
}

auto builtin_criterion_registration_name(const std::string_view aCriterionName) -> std::string
{
    return criterion_registration_name(input_parser::kBuiltinAppName, aCriterionName);
}

auto registered_criteria_names() -> std::set<std::string>
{
    return registered_criteria_names_impl(kFactoryRegistrationTypesSequence);
}

auto criterion_function_has_trait(const std::string_view aFunctionName, const CriterionTrait auto aTrait) -> bool
{
    const auto tToTraits =
        std::views::iota(0U, number_of_traits()) |
        std::views::transform([=](const auto aTraitIndex) { return traits_from_index(aTraitIndex); });

    return std::ranges::any_of(
        tToTraits, [=](const auto aTraits)
        { return has_trait(aTraits, aTrait) && criterion_function_has_traits(aFunctionName, aTraits); });
}

auto criterion_traits(const std::string_view aRegisteredCriterionName) -> std::vector<CriterionTraits>
{
    auto tToTraits = std::views::iota(0U, number_of_traits()) |
                     std::views::transform([](const auto aTraitIndex) { return traits_from_index(aTraitIndex); }) |
                     std::views::filter([=](const auto aTraits)
                                        { return criterion_function_has_traits(aRegisteredCriterionName, aTraits); }) |
                     std::views::common;

    return std::vector<CriterionTraits>{tToTraits.begin(), tToTraits.end()};
}

template auto criterion_function_has_trait(const std::string_view aFunctionName,
                                           const FunctionDimension aTrait) -> bool;
template auto criterion_function_has_trait(const std::string_view aFunctionName, const Parallelization aTrait) -> bool;

}  // namespace plato::criteria::library
