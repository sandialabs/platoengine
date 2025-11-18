#ifndef PLATO_CRITERIA_LIBRARY_CONSTRAINTVALIDATION
#define PLATO_CRITERIA_LIBRARY_CONSTRAINTVALIDATION

#include "plato/criteria/library/ConstraintInputBlock.hpp"

namespace plato::services
{
struct AppConfigurationWithDirectory;
}

namespace plato::criteria::library::detail
{
[[nodiscard]] auto validate_constraint_value(const input_parser::constraint& aInput) -> std::optional<std::string>;

[[nodiscard]] auto validate_constraint_number_of_processors(const input_parser::constraint& aInput)
    -> std::optional<std::string>;

[[nodiscard]] auto validate_constraint_type(const input_parser::constraint& aInput) -> std::optional<std::string>;

/// @brief Checks that, if the input contains a component-target constraint map, the criterion is a vector criterion and
/// provides a component map.
/// @param aAppConfigurations The set of app configuration files to use for validation. This function does not check the
/// plugins directory directly.
[[nodiscard]] auto validate_constraint_component_targets(
    const input_parser::constraint& aInput,
    const std::vector<services::AppConfigurationWithDirectory>& aAppConfigurations) -> std::optional<std::string>;

/// @brief Checks that, if the input contains a component-target constraint map, the criterion defines matching
/// component names.
/// @param aAppConfigurations The set of app configuration files to use for validation. This function does not check the
/// plugins directory directly.
[[nodiscard]] auto validate_constraint_component_names(
    const input_parser::constraint& aInput,
    const std::vector<services::AppConfigurationWithDirectory>& aAppConfigurations) -> std::optional<std::string>;

}  // namespace plato::criteria::library::detail

#endif
