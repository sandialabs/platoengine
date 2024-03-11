#include "plato/criteria/library/ObjectiveValidation.hpp"

#include <boost/mpi/communicator.hpp>

#include "plato/criteria/library/CriterionValidation.hpp"

namespace plato::criteria::library
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
        { return detail::validate_number_of_ranks_vs_objectives(aInput); }};

std::vector<std::string> validate_objectives(const std::vector<input_parser::objective>& aInput,
                                             std::vector<std::string>&& aCurrentMessageList)
{
    return detail::validate_criteria(aInput, std::move(aCurrentMessageList));
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

std::optional<std::string> validate_number_of_ranks_vs_objectives(const std::vector<input_parser::objective>& aInput)
{
    const auto tNumRanks = boost::mpi::communicator{}.size();
    if (tNumRanks < 0 || static_cast<std::size_t>(tNumRanks) > aInput.size())
    {
        return "The number of MPI ranks exceeds the number of objectives.";
    }
    else
    {
        return std::nullopt;
    }
}

}  // namespace detail
}  // namespace plato::criteria::library
