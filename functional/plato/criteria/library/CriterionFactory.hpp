#ifndef PLATO_CRITERIA_LIBRARY_CRITERIONFACTORY
#define PLATO_CRITERIA_LIBRARY_CRITERIONFACTORY

#include <concepts>
#include <string>

#include "plato/components/ComponentType.hpp"
#include "plato/core/Function.hpp"
#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::criteria::library
{
namespace detail
{
/// @brief Constrains templates to be instantiable only with either `input_parser::constraint` or
/// `input_parser::objective`.
template <typename Input>
concept CriterionInput = std::same_as<Input, input_parser::constraint> || std::same_as<Input, input_parser::objective>;
}  // namespace detail

/// @brief Converts either objective or constraint input objects to a common CriterionInput struct
/// @tparam Input Must be either input_parser::objective or input_parser::constraint input structs
template <detail::CriterionInput Input>
[[nodiscard]] auto to_criterion_input(const Input& aInput) -> CriterionInput;

/// @brief Creates a criterion Function object from either objective or constraint input objects.
/// @tparam Input Must be either input_parser::objective or input_parser::constraint input structs
template <typename FactoryReturn, typename InputBlockType, typename Input, typename... AdditionalArgs>
[[nodiscard]] auto make_criterion_function(const Input& aValidatedInput, const AdditionalArgs&... aArgs)
    -> FactoryReturn;

template <detail::CriterionInput Input>
auto to_criterion_input(const Input& aInput) -> CriterionInput
{
    return CriterionInput{.mNumberOfProcessors = aInput.number_of_processors.value_or(1),
                          .mInputFiles = aInput.input_files.value_or(input_parser::FileList{}),
                          .mComponentType = input_parser::ComponentTypeOfInputBlock<Input>::value,
                          .mName = aInput.name.value()};
}

template <typename FactoryReturn, typename InputBlockType, typename Input, typename... AdditionalArgs>
auto make_criterion_function(const Input& aValidatedInput, const AdditionalArgs&... aArgs) -> FactoryReturn
{
    static_assert(
        std::is_same_v<Input, input_validation::ValidatedInputDataBlock<components::ComponentType::kConstraint>> ||
            std::is_same_v<Input, input_validation::ValidatedInputDataBlock<components::ComponentType::kObjective>>,
        "make_criterion_function must only be called with input_parser::objective or "
        "input_parser::constraint wrapped in ValidatedInputTypeWrapper");

    const auto& tRawInput = input_validation::get_input_block<InputBlockType>(aValidatedInput);
    const auto tRegistrationName = criterion_registration_name(tRawInput.app, tRawInput.criterion.value());
    auto tCriterion = core::create_object_from_factory<FactoryReturn, CriterionInput, AdditionalArgs...>(
        tRegistrationName, to_criterion_input(tRawInput), aArgs...);
    if (tCriterion)
    {
        return std::move(tCriterion).value();
    }
    else
    {
        throw utilities::Exception("App/criterion \"" + tRegistrationName + "\" not found. Available criteria are:\n" +
                                   utilities::concatenate_container(registered_criteria_names(), "\n"));
    }
}
}  // namespace plato::criteria::library

#endif
