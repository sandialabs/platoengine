#ifndef PLATO_CRITERION_LIBRARY_OBJECTIVEVALIDATION
#define PLATO_CRITERION_LIBRARY_OBJECTIVEVALIDATION

#include "plato/core/ValidationRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"

namespace plato::criteria::library
{
/// @brief Validates all objective inputs in @a aInput, returning all error messages and appending to @a
/// aCurrentMessageList.
[[nodiscard]] std::vector<std::string> validate_objectives(const std::vector<input_parser::objective>& aInput,
                                                           std::vector<std::string>&& aCurrentMessageList);

/// @brief Returns `true` if any objective input in @a aInput has a value of more than one in its
///  `number_of_processors` field.
[[nodiscard]] bool has_parallel_objective(const std::vector<input_parser::objective>& aInput);

/// @brief Returns the total number of required ranks for all objectives specified in @a aInput.
///
/// Specifically, this sums all `number_of_processors` fields in each objective input.
[[nodiscard]] unsigned int total_number_of_processors(const std::vector<input_parser::objective>& aInput);

namespace detail
{
[[nodiscard]] std::optional<std::string> validate_aggregation_weight(const input_parser::objective& aInput);
[[nodiscard]] std::optional<std::string> validate_at_least_one_objective(
    const std::vector<input_parser::objective>& aInput);
[[nodiscard]] std::optional<std::string> validate_number_of_ranks_vs_serial_objectives(
    const std::vector<input_parser::objective>& aInput);
[[nodiscard]] std::optional<std::string> validate_number_of_ranks_vs_parallel_objectives(
    const std::vector<input_parser::objective>& aInput);
}  // namespace detail

}  // namespace plato::criteria::library

#endif
