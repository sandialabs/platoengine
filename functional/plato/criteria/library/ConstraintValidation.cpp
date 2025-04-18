#include "plato/criteria/library/ConstraintValidation.hpp"

#include <string>

#include "plato/criteria/library/CriterionValidation.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::criteria::library
{
namespace
{
[[maybe_unused]] static auto kConstraintValidationRegistration =
    input_validation::CrossReferencedInputValidationRegistration<>{
        [](const input_parser::new_constraint& aInput) { return detail::validate_criterion_is_registered(aInput); },
        [](const input_parser::new_constraint& aInput)
        { return detail::validate_constraint_number_of_processors(aInput); },
        [](const input_parser::new_constraint& aInput) { return detail::validate_constraint_value(aInput); },
        [](const input_parser::new_constraint& aInput) { return detail::validate_constraint_type(aInput); }};
}

namespace detail
{
auto validate_constraint_value(const input_parser::new_constraint& aInput) -> std::optional<std::string>
{
    return input_validation::error_message_for_empty_parameter(criterion_name(aInput), aInput.constraint_value,
                                                               "constraint_value");
}

auto validate_constraint_number_of_processors(const input_parser::new_constraint& aInput) -> std::optional<std::string>
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

auto validate_constraint_type(const input_parser::new_constraint& aInput) -> std::optional<std::string>
{
    auto tMessage = input_validation::error_message_for_empty_parameter(criterion_name(aInput), aInput.constraint_type,
                                                                        "constraint_type");
    if (tMessage.has_value())
    {
        const std::string tOptions = utilities::concatenate_container(
            utilities::all_strings_from_table(input_parser::kConstraintTypesTable), ", ");
        tMessage.value() = utilities::concatenate(tMessage.value(), ".\nSpecify one of: ", tOptions);
    }
    return tMessage;
}

}  // namespace detail

}  // namespace plato::criteria::library
