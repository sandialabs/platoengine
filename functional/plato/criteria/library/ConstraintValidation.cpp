#include "plato/criteria/library/ConstraintValidation.hpp"

#include <string>

#include "plato/criteria/library/CriterionValidation.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::criteria::library
{
[[maybe_unused]] static auto kConstraintValidationRegistration = core::ValidationRegistration<input_parser::constraint>{
    [](const input_parser::constraint& aInput) { return detail::validate_criterion_is_registered(aInput); },
    [](const input_parser::constraint& aInput) { return detail::validate_constraint_number_of_processors(aInput); },
    [](const input_parser::constraint& aInput) { return detail::validate_equal_to(aInput); }};

std::vector<std::string> validate_constraints(const std::vector<input_parser::constraint>& aInput,
                                              std::vector<std::string>&& aCurrentMessageList)
{
    return detail::validate_criteria(aInput, std::move(aCurrentMessageList));
}

namespace detail
{
std::optional<std::string> validate_equal_to(const input_parser::constraint& aInput)
{
    return core::error_message_for_empty_parameter(criterion_name(aInput), aInput.equal_to, "equal_to");
}

std::optional<std::string> validate_constraint_number_of_processors(const input_parser::constraint& aInput)
{
    constexpr auto kSupportedNumberOfProcessorsForConstraint = unsigned{1};
    if (aInput.number_of_processors.has_value() &&
        aInput.number_of_processors.value() != kSupportedNumberOfProcessorsForConstraint)
    {
        return std::optional<std::string>{
            utilities::concatenate("Constraints currently only support serial evaluation. ", criterion_name(aInput),
                                   " requested ", aInput.number_of_processors.value(), " number_of_processors.")};
    }
    else
    {
        return std::nullopt;
    }
}

}  // namespace detail

}  // namespace plato::criteria::library