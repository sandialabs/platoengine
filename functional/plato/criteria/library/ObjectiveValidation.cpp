#include "plato/criteria/library/ObjectiveValidation.hpp"

#include <algorithm>
#include <boost/mpi/communicator.hpp>
#include <cstddef>
#include <numeric>
#include <optional>
#include <string>
#include <vector>

#include "plato/criteria/library/CriterionValidation.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/utilities/BoundClusivity.hpp"
#include "plato/utilities/ParameterBounds.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::criteria::library
{
namespace
{
[[maybe_unused]] static const auto kObjectiveValidationRegistration =
    input_validation::InputBlockValidationRegistration<>{
        [](const input_parser::objective& aInput) { return detail::validate_criterion_is_registered(aInput); },
        [](const input_parser::objective& aInput) { return detail::validate_number_of_processors(aInput); },
        [](const input_parser::objective& aInput) { return detail::validate_aggregation_weight(aInput); },
        [](const input_parser::objective& aInput) { return detail::validate_criterion_files_exist(aInput); },
        [](const input_parser::objective& aInput)
        { return detail::validate_criterion_supports_requested_number_of_processors(aInput); }};

[[maybe_unused]] static const auto kListObjectivesValidationRegistration =
    input_validation::ParsedInputValidationRegistration<>{
        [](const input_parser::ParsedInput& aInput)
        { return detail::validate_at_least_one_objective(aInput.get<input_parser::objective>()); },
        [](const input_parser::ParsedInput& aInput)
        { return detail::validate_number_of_ranks_vs_serial_objectives(aInput.get<input_parser::objective>()); },
        [](const input_parser::ParsedInput& aInput)
        { return detail::validate_number_of_ranks_vs_parallel_objectives(aInput.get<input_parser::objective>()); }};

auto number_of_processors(const input_parser::objective& aObjective) -> unsigned int
{
    return input_validation::is_active(aObjective) ? aObjective.number_of_processors.value_or(1u) : 0u;
}
}  // namespace

auto has_parallel_objective(const std::vector<input_parser::objective>& aInput) -> bool
{
    return std::any_of(aInput.begin(), aInput.end(),
                       [](const auto& aObjectiveInput) { return number_of_processors(aObjectiveInput) > 1u; });
}

auto total_number_of_processors(const std::vector<input_parser::objective>& aInput) -> unsigned int
{
    return std::accumulate(aInput.begin(), aInput.end(), 0u, [](const unsigned int aTotal, const auto& aObjectiveInput)
                           { return aTotal + number_of_processors(aObjectiveInput); });
}

auto number_of_active_objectives(const std::vector<input_parser::objective>& aInput) -> unsigned int
{
    return std::accumulate(aInput.begin(), aInput.end(), 0U, [](const auto aTotalActive, const auto& aObjective)
                           { return aTotalActive + (input_validation::is_active(aObjective) ? 1U : 0U); });
}

namespace detail
{
auto validate_aggregation_weight(const input_parser::objective& aInput) -> std::optional<std::string>
{
    namespace pfu = plato::utilities;
    return input_validation::error_message_for_parameter_out_of_bounds(criterion_name(aInput),
                                                                       aInput.aggregation_weight, "aggregation_weight",
                                                                       pfu::lower_bounded(pfu::Exclusive{0.0}));
}

auto validate_at_least_one_objective(const std::vector<input_parser::objective>& aInput) -> std::optional<std::string>
{
    const bool tAnyActiveObjectives =
        std::any_of(aInput.begin(), aInput.end(),
                    [](const input_parser::objective& aObjective) { return input_validation::is_active(aObjective); });

    if (tAnyActiveObjectives)
    {
        return std::nullopt;
    }
    else
    {
        return "No active objectives found.";
    }
}

auto validate_number_of_ranks_vs_serial_objectives(const std::vector<input_parser::objective>& aInput)
    -> std::optional<std::string>
{
    const auto tNumRanks = static_cast<std::size_t>(boost::mpi::communicator{}.size());
    if (!has_parallel_objective(aInput) && static_cast<std::size_t>(tNumRanks) > number_of_active_objectives(aInput))
    {
        return std::optional<std::string>{utilities::concatenate(
            "The number of MPI ranks exceeds the number of objectives.\n Number of ranks: ", tNumRanks,
            "\n Number of processors needed for objectives: ", number_of_active_objectives(aInput))};
    }
    else
    {
        return std::nullopt;
    }
}

auto validate_number_of_ranks_vs_parallel_objectives(const std::vector<input_parser::objective>& aInput)
    -> std::optional<std::string>
{
    const auto tNumRanks = static_cast<std::size_t>(boost::mpi::communicator{}.size());
    const auto tTotalProcessors = total_number_of_processors(aInput);
    if (has_parallel_objective(aInput) && tTotalProcessors != tNumRanks)
    {
        return std::optional<std::string>{utilities::concatenate(
            "The number of MPI ranks must match the number of requested processors for parallelized "
            "objectives.\n Number of ranks: ",
            tNumRanks, "\n Number of processors needed for objectives: ", tTotalProcessors)};
    }
    else
    {
        return std::nullopt;
    }
}

}  // namespace detail
}  // namespace plato::criteria::library
