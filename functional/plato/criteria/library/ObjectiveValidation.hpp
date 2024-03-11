#ifndef PLATO_CRITERION_LIBRARY_OBJECTIVEVALIDATION
#define PLATO_CRITERION_LIBRARY_OBJECTIVEVALIDATION

#include "plato/core/ValidationRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"

namespace plato::criteria::library
{
namespace detail
{
[[nodiscard]] std::optional<std::string> validate_aggregation_weight(const input_parser::objective& aInput);
[[nodiscard]] std::optional<std::string> validate_at_least_one_objective(
    const std::vector<input_parser::objective>& aInput);
[[nodiscard]] std::optional<std::string> validate_number_of_ranks_vs_objectives(
    const std::vector<input_parser::objective>& aInput);
}  // namespace detail

[[nodiscard]] std::vector<std::string> validate_objectives(const std::vector<input_parser::objective>& aInput,
                                                           std::vector<std::string>&& aCurrentMessageList);

}  // namespace plato::criteria::library

#endif