#ifndef PLATO_CRITERIA_LIBRARY_CRITERIONVALIDATION
#define PLATO_CRITERIA_LIBRARY_CRITERIONVALIDATION

#include <optional>

#include "plato/core/ValidationRegistration.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"

namespace plato::criteria::library
{
namespace detail
{
template <typename Criteria>
[[nodiscard]] std::string criterion_name(const Criteria& aInput)
{
    const std::string tName = aInput.name.value_or("unnamed");
    return input_parser::block_name<Criteria>() + " " + tName;
}

template <typename Criteria>
[[nodiscard]] std::optional<std::string> validate_criterion_is_registered(const Criteria& aInput)
{
    if (aInput.criterion.has_value())
    {
        const auto tRegistrationName = criterion_registration_name(aInput.app, aInput.criterion.value());
        if (is_criterion_function_registered(tRegistrationName) ||
            is_parallel_criterion_function_registered(tRegistrationName))
        {
            return std::nullopt;
        }
        else
        {
            auto tErrorMessage = criterion_name(aInput) + ": app/criterion not found: " + tRegistrationName +
                                 "\nThe following criteria are available: \n" +
                                 utilities::concatenate_container(registered_criteria_names(), "\n");
            return std::optional<std::string>{std::in_place_t{}, std::move(tErrorMessage)};
        }
    }
    else
    {
        return core::error_message_for_empty_parameter(criterion_name(aInput), aInput.criterion, "criterion");
    }
}

template <typename Criteria>
[[nodiscard]] std::optional<std::string> validate_number_of_processors(const Criteria& aInput)
{
    return core::error_message_for_optional_parameter_out_of_bounds(criterion_name(aInput), aInput.number_of_processors,
                                                                    "number_of_processors",
                                                                    utilities::lower_bounded(utilities::Inclusive{1u}));
}

template <typename Criteria>
[[nodiscard]] std::vector<std::string> validate_criteria(const std::vector<Criteria>& aInput,
                                                         std::vector<std::string>&& aCurrentMessageList)
{
    aCurrentMessageList = core::validate(aInput, std::move(aCurrentMessageList));
    for (const auto& iCriterionInput : aInput)
    {
        aCurrentMessageList = core::validate(iCriterionInput, std::move(aCurrentMessageList));
    }
    return std::move(aCurrentMessageList);
}

}  // namespace detail

}  // namespace plato::criteria::library

#endif
