#ifndef PLATO_CRITERIA_LIBRARY_CRITERIONVALIDATION
#define PLATO_CRITERIA_LIBRARY_CRITERIONVALIDATION

#include "plato/core/ValidationRegistration.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/input_parser/InputBlocks.hpp"

namespace plato::criteria::library
{
namespace detail
{
template <typename Criteria>
[[nodiscard]] std::string criterion_name(const Criteria& aInput)
{
    std::string tName = aInput.name.value_or("unnamed");
    return input_parser::block_name<Criteria>() + " " + tName;
}

template <typename Criteria>
[[nodiscard]] std::optional<std::string> validate_app(const Criteria& aInput)
{
    return core::error_message_for_empty_parameter(criterion_name(aInput), aInput.app, "app");
}

template <typename Criteria>
[[nodiscard]] std::optional<std::string> validate_custom_app(const Criteria& aInput)
{
    if (aInput.app.has_value() && aInput.app.value() == input_parser::CodeOptions::kCustomApp)
    {
        return core::error_message_for_empty_parameter(criterion_name(aInput), aInput.shared_library_path,
                                                       "shared_library_path");
    }
    else
    {
        return std::nullopt;
    }
}

template <typename Criteria>
[[nodiscard]] std::optional<std::string> validate_number_of_processors(const Criteria& aInput)
{
    namespace pfu = plato::utilities;
    if (aInput.number_of_processors.has_value())
    {
        return core::error_message_for_parameter_out_of_bounds(criterion_name(aInput), aInput.number_of_processors,
                                                               "num_processors",
                                                               pfu::lower_bounded(pfu::Inclusive{1u}));
    }
    else
    {
        return std::nullopt;
    }
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