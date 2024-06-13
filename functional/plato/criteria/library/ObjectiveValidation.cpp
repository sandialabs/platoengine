#include "plato/criteria/library/ObjectiveValidation.hpp"

#include <algorithm>
#include <boost/mpi/communicator.hpp>
#include <numeric>
#include <optional>

#include "plato/core/ValidationUtilities.hpp"
#include "plato/criteria/library/CriterionValidation.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::criteria::library
{
namespace
{
[[maybe_unused]] static auto kObjectiveValidationRegistration = core::ValidationRegistration<input_parser::objective>{
    [](const input_parser::objective& aInput) { return detail::validate_app(aInput); },
    [](const input_parser::objective& aInput) { return detail::validate_custom_app(aInput); },
    [](const input_parser::objective& aInput) { return detail::validate_number_of_processors(aInput); },
    [](const input_parser::objective& aInput) { return detail::validate_aggregation_weight(aInput); }};

[[maybe_unused]] static auto kListObjectivesValidationRegistration =
    core::ValidationRegistration<std::vector<input_parser::objective>>{
        [](const std::vector<input_parser::objective>& aInput)
        { return detail::validate_at_least_one_objective(aInput); },
        [](const std::vector<input_parser::objective>& aInput)
        { return detail::validate_number_of_ranks_vs_serial_objectives(aInput); },
        [](const std::vector<input_parser::objective>& aInput)
        { return detail::validate_number_of_ranks_vs_parallel_objectives(aInput); }};

unsigned int number_of_processors(const input_parser::objective& aObjective)
{
    return core::is_active(aObjective) ? aObjective.number_of_processors.value_or(1u) : 0u;
}
}  // namespace

std::vector<std::string> validate_objectives(const std::vector<input_parser::objective>& aInput,
                                             std::vector<std::string>&& aCurrentMessageList)
{
    return detail::validate_criteria(aInput, std::move(aCurrentMessageList));
}

bool has_parallel_objective(const std::vector<input_parser::objective>& aInput)
{
    return std::any_of(aInput.begin(), aInput.end(),
                       [](const auto& aObjectiveInput) { return number_of_processors(aObjectiveInput) > 1u; });
}

unsigned int total_number_of_processors(const std::vector<input_parser::objective>& aInput)
{
    return std::accumulate(aInput.begin(), aInput.end(), 0u,
                           [](const unsigned int aTotal, const auto& aObjectiveInput)
                           { return aTotal + number_of_processors(aObjectiveInput); });
}

namespace detail
{
std::optional<std::string> validate_aggregation_weight(const input_parser::objective& aInput)
{
    namespace pfu = plato::utilities;
    return core::error_message_for_parameter_out_of_bounds(criterion_name(aInput), aInput.aggregation_weight,
                                                           "aggregation_weight",
                                                           pfu::lower_bounded(pfu::Exclusive{0.0}));
}

std::optional<std::string> validate_at_least_one_objective(const std::vector<input_parser::objective>& aInput)
{
    const bool tAnyActiveObjectives =
        std::any_of(aInput.begin(), aInput.end(),
                    [](const input_parser::objective& aObjective) { return core::is_active(aObjective); });

    if (tAnyActiveObjectives)
    {
        return std::nullopt;
    }
    else
    {
        return "No active objectives found.";
    }
}

std::optional<std::string> validate_number_of_ranks_vs_serial_objectives(const std::vector<input_parser::objective>& aInput)
{
    const auto tNumRanks = static_cast<std::size_t>(boost::mpi::communicator{}.size());
    if (!has_parallel_objective(aInput) && static_cast<std::size_t>(tNumRanks) > aInput.size())
    {
        return std::optional<std::string>{utilities::concatenate(
            "The number of MPI ranks exceeds the number of objectives.\n Number of ranks: ", tNumRanks,
            "\n Number of processors needed for objectives: ", aInput.size())};
    }
    else
    {
        return std::nullopt;
    }
}

std::optional<std::string> validate_number_of_ranks_vs_parallel_objectives(
    const std::vector<input_parser::objective>& aInput)
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
