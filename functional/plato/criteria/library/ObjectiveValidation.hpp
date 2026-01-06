#ifndef PLATO_CRITERION_LIBRARY_OBJECTIVEVALIDATION
#define PLATO_CRITERION_LIBRARY_OBJECTIVEVALIDATION

#include <optional>
#include <string>

#include "plato/criteria/library/ObjectiveInputBlock.hpp"

namespace plato::criteria::library
{
/// @brief Returns `true` if any objective input in @a aInput has a value of more than one in its
///  `number_of_processors` field.
[[nodiscard]] auto has_parallel_objective(const std::vector<input_parser::objective>& aInput) -> bool;

/// @brief Returns the total number of required ranks for all objectives specified in @a aInput.
///
/// Specifically, this sums all `number_of_processors` fields in each objective input.
[[nodiscard]] auto total_number_of_processors(const std::vector<input_parser::objective>& aInput) -> unsigned int;

/// @brief Returns the total number of active objectives
[[nodiscard]] auto number_of_active_objectives(const std::vector<input_parser::objective>& aInput) -> unsigned int;

namespace detail
{
[[nodiscard]] auto validate_aggregation_weight(const input_parser::objective& aInput) -> std::optional<std::string>;
[[nodiscard]] auto validate_at_least_one_objective(const std::vector<input_parser::objective>& aInput)
    -> std::optional<std::string>;
[[nodiscard]] auto validate_number_of_ranks_vs_serial_objectives(const std::vector<input_parser::objective>& aInput)
    -> std::optional<std::string>;
[[nodiscard]] auto validate_number_of_ranks_vs_parallel_objectives(const std::vector<input_parser::objective>& aInput)
    -> std::optional<std::string>;
}  // namespace detail

}  // namespace plato::criteria::library

#endif
