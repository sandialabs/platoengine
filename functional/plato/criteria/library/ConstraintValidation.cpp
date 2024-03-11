#include "plato/criteria/library/ConstraintValidation.hpp"

#include "plato/criteria/library/CriterionValidation.hpp"

namespace plato::criteria::library
{
[[maybe_unused]] static auto kConstraintValidationRegistration = core::ValidationRegistration<input_parser::constraint>{
    [](const input_parser::constraint& aInput) { return detail::validate_app(aInput); },
    [](const input_parser::constraint& aInput) { return detail::validate_custom_app(aInput); },
    [](const input_parser::constraint& aInput) { return detail::validate_number_of_processors(aInput); },
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

}  // namespace detail

}  // namespace plato::criteria::library