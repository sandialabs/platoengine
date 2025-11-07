#include "plato/criteria/library/ConstraintValidation.hpp"

#include <string>

#include "plato/criteria/library/CriterionValidation.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/services/AppConfigurationUtilities.hpp"
#include "plato/services/PluginDirectoryPath.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::criteria::library
{
namespace
{
const auto kBindWithPluginDirectory = [](const auto& aValidationFunction,
                                         const input_parser::constraint& aInput) -> std::optional<std::string>
{
    if (const auto tPluginDirectory = services::plugin_directory_path())
    {
        return aValidationFunction(aInput, services::app_configurations({tPluginDirectory.value()}));
    }
    return std::nullopt;
};

[[maybe_unused]] static auto kConstraintValidationRegistration = input_validation::InputBlockValidationRegistration<>{
    [](const input_parser::constraint& aInput) { return detail::validate_criterion_is_registered(aInput); },
    [](const input_parser::constraint& aInput) { return detail::validate_constraint_number_of_processors(aInput); },
    [](const input_parser::constraint& aInput) { return detail::validate_constraint_value(aInput); },
    [](const input_parser::constraint& aInput) { return detail::validate_constraint_type(aInput); },
    [](const input_parser::constraint& aInput)
    { return kBindWithPluginDirectory(detail::validate_constraint_component_names, aInput); },
    [](const input_parser::constraint& aInput)
    { return kBindWithPluginDirectory(detail::validate_constraint_component_targets, aInput); }};

/// @brief Returns an app/criterion name combination for error messages.
[[nodiscard]] auto app_criterion_name(const input_parser::constraint& aConstraintInput) -> std::string
{
    return std::string{aConstraintInput.app.value_or(input_parser::AppName{"undefined"}).mToken} + ":" +
           std::string{aConstraintInput.criterion.value_or(input_parser::CriterionName{"undefined"}).mToken};
}

/// @brief Checks that the app/criterion combination exists in the set of configured apps given by @a
/// aAppConfigurations.
[[nodiscard]] auto criterion_exists(const boost::optional<input_parser::AppName>& aAppName,
                                    const boost::optional<input_parser::CriterionName>& aCriterionName,
                                    const std::vector<services::AppConfigurationWithDirectory>& aAppConfigurations)
    -> bool
{
    if (!aAppName || !aCriterionName)
    {
        return false;
    }
    return services::criterion_configuration_with_name(
               services::CriterionName{.mAppName = aAppName->mToken, .mCriterionName = aCriterionName->mToken},
               aAppConfigurations)
        .has_value();
}

[[nodiscard]] auto component_exists(const services::CriterionConfiguration& aConfiguration,
                                    const std::string_view aComponentName) -> bool
{
    return aConfiguration.mVectorComponents.has_value() &&
           std::ranges::any_of(aConfiguration.mVectorComponents.value(),
                               [&aComponentName](const auto& aComponentIndexAndName)
                               { return aComponentName == aComponentIndexAndName.second; });
}

[[nodiscard]] auto component_names_for_error(const std::map<std::size_t, std::string>& aComponentIndexToName)
    -> std::string
{
    return std::accumulate(aComponentIndexToName.begin(), aComponentIndexToName.end(), std::string{},
                           [](std::string&& aAllComponents, const auto& aComponentIndexAndName)
                           { return aAllComponents += aComponentIndexAndName.second + "\n"; });
}

}  // namespace

namespace detail
{
auto validate_constraint_value(const input_parser::constraint& aInput) -> std::optional<std::string>
{
    if (aInput.constraint_value && aInput.constraint_value_list)
    {
        return std::optional<std::string>{utilities::concatenate(
            criterion_name(aInput),
            " defines both scalar and vector constraint targets. Only one target definition is allowed.")};
    }
    else if (!aInput.constraint_value && !aInput.constraint_value_list)
    {
        return std::optional<std::string>{utilities::concatenate(
            criterion_name(aInput),
            " does not define constraint_value or constraint_value_list. One constraint value entry is required.")};
    }
    return std::nullopt;
}

auto validate_constraint_number_of_processors(const input_parser::constraint& aInput) -> std::optional<std::string>
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

auto validate_constraint_type(const input_parser::constraint& aInput) -> std::optional<std::string>
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

auto validate_constraint_component_targets(
    const input_parser::constraint& aInput,
    const std::vector<services::AppConfigurationWithDirectory>& aAppConfigurations) -> std::optional<std::string>
{
    if (criterion_exists(aInput.app, aInput.criterion, aAppConfigurations))
    {
        const auto tConfiguration = services::criterion_configuration_with_name(
            services::CriterionName{.mAppName = aInput.app->mToken, .mCriterionName = aInput.criterion->mToken},
            aAppConfigurations);

        if (aInput.constraint_value_list.has_value() && !tConfiguration->mVectorComponents.has_value())
        {
            return utilities::concatenate(
                criterion_name(aInput), R"(: Vector constraint targets were defined in the input, but the criterion ")",
                app_criterion_name(aInput), R"(" does not define any vector components)");
        }

        if (tConfiguration->mVectorComponents.has_value() && !aInput.constraint_value_list.has_value())
        {
            return utilities::concatenate(criterion_name(aInput), R"(: Vector criterion ")", app_criterion_name(aInput),
                                          R"(" defines vector components, but no vector targets were defined.)");
        }
    }
    return std::nullopt;
}

auto validate_constraint_component_names(const input_parser::constraint& aInput,
                                         const std::vector<services::AppConfigurationWithDirectory>& aAppConfigurations)
    -> std::optional<std::string>
{
    if (criterion_exists(aInput.app, aInput.criterion, aAppConfigurations) && aInput.constraint_value_list)
    {
        const auto tConfiguration = services::criterion_configuration_with_name(
            services::CriterionName{.mAppName = aInput.app->mToken, .mCriterionName = aInput.criterion->mToken},
            aAppConfigurations);

        const auto tAllDefined = std::ranges::all_of(
            aInput.constraint_value_list->mList, [&tConfiguration](const auto& aComponentAndTarget)
            { return component_exists(tConfiguration.value(), aComponentAndTarget.component.mToken); });
        if (!tAllDefined)
        {
            return utilities::concatenate(criterion_name(aInput),
                                          ": Vector constraint target components do not match those defined in the "
                                          "criterion. Valid component names are:\n",
                                          component_names_for_error(tConfiguration.value().mVectorComponents.value()));
        }
    }
    return std::nullopt;
}
}  // namespace detail

}  // namespace plato::criteria::library
