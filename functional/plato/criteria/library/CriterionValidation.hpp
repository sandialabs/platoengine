#ifndef PLATO_CRITERIA_LIBRARY_CRITERIONVALIDATION
#define PLATO_CRITERIA_LIBRARY_CRITERIONVALIDATION

#include <format>
#include <optional>

#include "plato/criteria/library/CriterionInput.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::criteria::library::detail
{

/// @brief Returns a name for the criterion in @a aInput useful for error messages.
template <CriterionInput Criteria>
[[nodiscard]] auto criterion_name(const Criteria& aInput) -> std::string;

/// @brief Helper function that validates @a aInput input_files. If a list is specified, the files must exist on disk.
template <CriterionInput Criteria>
[[nodiscard]] auto validate_criterion_files_exist(const Criteria& aInput) -> std::optional<std::string>;

/// @brief Validates that a criterion is registered.
template <CriterionInput Criteria>
[[nodiscard]] auto validate_criterion_is_registered(const Criteria& aInput) -> std::optional<std::string>;

/// @brief Validates that, if specified, the number of processors is greater than 0.
template <CriterionInput Criteria>
[[nodiscard]] auto validate_number_of_processors(const Criteria& aInput) -> std::optional<std::string>;

/// @brief Validates that the input files specified in @a aInput exist on disk.
template <CriterionInput Criteria>
[[nodiscard]] auto validate_criterion_files_exist(const Criteria& aInput) -> std::optional<std::string>;

/// @brief Checks that, if more than one processor is requested, that the criterion is registered as being parallelized.
template <CriterionInput Criteria>
[[nodiscard]] auto validate_criterion_supports_requested_number_of_processors(const Criteria& aInput)
    -> std::optional<std::string>;

template <CriterionInput Criteria>
[[nodiscard]] auto criterion_name(const Criteria& aInput) -> std::string
{
    const std::string tName = aInput.name.value_or("unnamed");
    return input_parser::block_name<Criteria>() + " " + tName;
}

template <CriterionInput Criteria>
[[nodiscard]] auto validate_criterion_is_registered(const Criteria& aInput) -> std::optional<std::string>
{
    if (aInput.criterion.has_value())
    {
        const auto tRegistrationName = criterion_registration_name(aInput.app, aInput.criterion.value());
        if (is_criterion_function_registered(tRegistrationName))
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
        return input_validation::error_message_for_empty_parameter(criterion_name(aInput), aInput.criterion,
                                                                   "criterion");
    }
}

template <CriterionInput Criteria>
[[nodiscard]] auto validate_number_of_processors(const Criteria& aInput) -> std::optional<std::string>
{
    return input_validation::error_message_for_optional_parameter_out_of_bounds(
        criterion_name(aInput), aInput.number_of_processors, "number_of_processors",
        utilities::lower_bounded(utilities::Inclusive{1u}));
}

template <CriterionInput Criteria>
[[nodiscard]] auto validate_criterion_files_exist(const Criteria& aInput) -> std::optional<std::string>
{
    if (aInput.input_files.has_value())
    {
        auto tMissingFiles = utilities::reserved_container<std::vector<std::string>>(aInput.input_files->size());
        std::ranges::copy_if(aInput.input_files.value(), std::back_inserter(tMissingFiles),
                             [](const auto& aFileName) { return !std::filesystem::exists(aFileName); });

        if (!tMissingFiles.empty())
        {
            return criterion_name(aInput) +
                   ": app/criterion could not find all files listed in 'input_files'. The following files were "
                   "missing: " +
                   utilities::concatenate_container(tMissingFiles);
        }
    }
    return std::nullopt;
}

/// @brief Checks that, if more than one processor is requested, that the criterion is registered as being parallelized.
template <CriterionInput Criteria>
[[nodiscard]] auto validate_criterion_supports_requested_number_of_processors(const Criteria& aInput)
    -> std::optional<std::string>
{
    if (aInput.criterion &&
        is_criterion_function_registered(criterion_registration_name(aInput.app, aInput.criterion.value())))
    {
        const auto tRegistrationName = criterion_registration_name(aInput.app, aInput.criterion.value());
        const auto tNumberOfProcessorsRequested = aInput.number_of_processors.value_or(1U);
        const auto tParallelAndNotRegistered =
            tNumberOfProcessorsRequested > 1U &&
            !criterion_function_has_trait(tRegistrationName, Parallelization::kParallel);
        const auto tSerialAndNotRegistered = tNumberOfProcessorsRequested == 1U &&
                                             !criterion_function_has_trait(tRegistrationName, Parallelization::kSerial);
        if (tParallelAndNotRegistered || tSerialAndNotRegistered)
        {
            auto tErrorMessage =
                std::format("{}: app:criterion ({}) does not support requested number of processors ({}).",
                            criterion_name(aInput), tRegistrationName, tNumberOfProcessorsRequested);
            return std::optional<std::string>{std::move(tErrorMessage)};
        }
    }
    return std::nullopt;
}

}  // namespace plato::criteria::library::detail

#endif
