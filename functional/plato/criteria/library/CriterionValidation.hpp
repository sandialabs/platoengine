#ifndef PLATO_CRITERIA_LIBRARY_CRITERIONVALIDATION
#define PLATO_CRITERIA_LIBRARY_CRITERIONVALIDATION

#include <format>
#include <optional>

#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::criteria::library::detail
{
template <typename Criteria>
[[nodiscard]] std::string criterion_name(const Criteria& aInput)
{
    const std::string tName = aInput.name.value_or("unnamed");
    return input_parser::block_name<Criteria>() + " " + tName;
}

/// @brief Helper function that validates @a aInput input_files. If a list is specified, the files must exist on disk.
template <typename Criteria>
[[nodiscard]] auto validate_criterion_files_exist(const Criteria& aInput) -> std::optional<std::string>;

template <typename Criteria>
[[nodiscard]] std::optional<std::string> validate_criterion_is_registered(const Criteria& aInput)
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

template <typename Criteria>
[[nodiscard]] std::optional<std::string> validate_number_of_processors(const Criteria& aInput)
{
    return input_validation::error_message_for_optional_parameter_out_of_bounds(
        criterion_name(aInput), aInput.number_of_processors, "number_of_processors",
        utilities::lower_bounded(utilities::Inclusive{1u}));
}

template <typename Criteria>
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
template <typename Criteria>
[[nodiscard]] auto validate_criterion_supports_requested_number_of_processors(const Criteria& aInput)
    -> std::optional<std::string>
{
    if (aInput.criterion)
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
            auto tErrorMessage = std::format("{}: app/criterion dose not support requested number of processors ({}), or it does not exist.\n",
                                             criterion_name(aInput), tNumberOfProcessorsRequested);
            return std::optional<std::string>{std::move(tErrorMessage)};
        }
    }
    return std::nullopt;
}

}  // namespace plato::criteria::library::detail

#endif
