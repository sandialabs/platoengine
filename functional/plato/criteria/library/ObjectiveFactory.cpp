#include "plato/criteria/library/ObjectiveFactory.hpp"

#include <algorithm>
#include <boost/mpi/communicator.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <iterator>
#include <numeric>

#include "plato/core/ParallelFunction.hpp"
#include "plato/criteria/library/CriterionFactory.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/criteria/library/ObjectiveReciprocal.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/linear_algebra/DynamicVectorSerialization.hpp"
#include "plato/utilities/MPIUtilities.hpp"
#include "plato/utilities/NamedType.hpp"
#include "plato/utilities/RankSplitVector.hpp"
#include "plato/utilities/ReduceUtilities.hpp"
#include "plato/utilities/TransformIf.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::criteria::library
{
namespace detail
{
namespace
{
using ValidatedObjective = input_validation::ValidatedInputDataBlock<components::ComponentType::kObjective>;
using ObjectiveComm = utilities::NamedType<boost::mpi::communicator, struct ObjectiveCommTag>;

const auto kIsActive = [](const auto& aObjective)
{ return input_validation::is_active(input_validation::get_input_block<input_parser::objective>(aObjective)); };

const auto kNormalizeByInitialValue = [](const auto& aObjective) -> bool
{
    const auto& tInput = input_validation::get_input_block<input_parser::objective>(aObjective);
    return input_validation::is_active(tInput) && tInput.normalize_by_initial_value.value_or(false);
};

[[nodiscard]] auto objective_goal(const ValidatedObjective& aObjective) -> ObjectiveGoal
{
    return input_validation::get_input_block<input_parser::objective>(aObjective)
        .objective_goal.value_or(ObjectiveGoal::kMinimize);
}

[[nodiscard]] auto objective_goal_scaling(const ValidatedObjective& aObjective) -> double
{
    return objective_goal(aObjective) == ObjectiveGoal::kMinimizeNegation ? -1.0 : 1.0;
}

[[nodiscard]] bool is_parallel_objective(const ValidatedObjective& aObjective)
{
    return input_validation::get_input_block<input_parser::objective>(aObjective).number_of_processors.value_or(1U) >
           1U;
}

[[nodiscard]] auto make_parallel_criterion_function(const ValidatedObjective& aObjective,
                                                    const ObjectiveComm& aObjectiveComm)
{
    if (is_parallel_objective(aObjective))
    {
        return core::adapt_parallel_function(
            make_reciprocal_criterion_function(
                make_criterion_function<CriterionFunction, input_parser::objective>(aObjective, aObjectiveComm.mValue)
                    .mFunction,
                objective_goal(aObjective)),
            aObjectiveComm.mValue);
    }
    else
    {
        return make_reciprocal_criterion_function(
            make_criterion_function<CriterionFunction, input_parser::objective>(aObjective).mFunction,
            objective_goal(aObjective));
    }
}

[[nodiscard]] auto active_parallel_objectives(const std::vector<ValidatedObjective>& aObjectiveInputBlocks,
                                              const ObjectiveComm& aObjectiveComm) -> std::vector<ObjectiveFunction>
{
    std::vector<ObjectiveFunction> tObjectives;
    utilities::transform_if(
        aObjectiveInputBlocks, std::back_inserter(tObjectives), [&aObjectiveComm](const auto& aObjective)
        { return make_parallel_criterion_function(aObjective, aObjectiveComm); }, kIsActive);
    return tObjectives;
}

[[nodiscard]] auto make_parallel_aggregate_impl(const std::vector<ValidatedObjective>& aObjectiveInputBlocks,
                                                const AggregateComm& aAggregatorComm,
                                                const ObjectiveComm& aObjectiveComm) -> ParallelAggregateObjective
{
    using ObjectiveAndWeight = std::pair<ObjectiveFunction, double>;
    std::vector<ObjectiveAndWeight> tFunctionsAndWeights;
    utilities::transform_if(
        aObjectiveInputBlocks, std::back_inserter(tFunctionsAndWeights),
        [&aObjectiveComm](const auto& aObjective)
        {
            const auto tWeight =
                input_validation::get_input_block<input_parser::objective>(aObjective).aggregation_weight.value() *
                objective_goal_scaling(aObjective);
            return std::make_pair(make_parallel_criterion_function(aObjective, aObjectiveComm), tWeight);
        },
        kIsActive);
    return ParallelAggregateObjective{std::move(tFunctionsAndWeights), aAggregatorComm.mValue};
}

[[nodiscard]] auto make_normalized_parallel_aggregate_impl(const std::vector<ValidatedObjective>& aObjectiveInputBlocks,
                                                           const AggregateComm& aAggregatorComm,
                                                           const ObjectiveComm& aObjectiveComm,
                                                           const std::vector<double>& aInitialValues)
    -> ParallelAggregateObjective
{
    using ObjectiveAndWeight = std::pair<ObjectiveFunction, double>;
    std::vector<ValidatedObjective> tActiveObjectives{};
    std::ranges::copy_if(aObjectiveInputBlocks, std::back_inserter(tActiveObjectives), kIsActive);

    std::vector<ObjectiveAndWeight> tFunctionsAndWeights;
    for (const auto& [tObjectiveInput, tInitialValue] : utilities::Zip{tActiveObjectives, aInitialValues})
    {
        const auto tInitialNormalization = kNormalizeByInitialValue(tObjectiveInput) ? 1.0 / tInitialValue : 1.0;
        const auto tWeight =
            tInitialNormalization *
            input_validation::get_input_block<input_parser::objective>(tObjectiveInput).aggregation_weight.value() *
            objective_goal_scaling(tObjectiveInput);
        tFunctionsAndWeights.push_back(
            std::make_pair(make_parallel_criterion_function(tObjectiveInput, aObjectiveComm), tWeight));
    }
    return ParallelAggregateObjective{std::move(tFunctionsAndWeights), aAggregatorComm.mValue};
}

[[nodiscard]] auto objective_group_color_and_size(const ValidatedObjectives& aInput,
                                                  const boost::mpi::communicator& aComm)
{
    const auto tNumberOfProcessors = number_of_processors_per_objective(aInput);
    const auto tGroupColor = utilities::rank_group_color(tNumberOfProcessors, utilities::RankNamedType{aComm.rank()});
    const auto tNumberOfGroups = boost::numeric_cast<int>(tNumberOfProcessors.size());
    const auto tSplitSize = std::min(aComm.size(), tNumberOfGroups);
    return std::make_pair(tGroupColor, tSplitSize);
}

[[nodiscard]] auto mpi_group(const ValidatedObjectives& aInput, const boost::mpi::communicator& aComm)
    -> boost::mpi::communicator
{
    const auto tNumberOfProcessors = number_of_processors_per_objective(aInput);
    const auto tGroupColor = utilities::rank_group_color(tNumberOfProcessors, utilities::RankNamedType{aComm.rank()});
    return aComm.split(tGroupColor.mValue);
}

auto make_normalized_parallel_aggregate(const ValidatedObjectives& aInput, const std::vector<double>& aInitialValues)
    -> ParallelAggregateObjective
{
    const auto tCommunicator = boost::mpi::communicator{};
    const auto [tGroupColor, tSplitSize] = objective_group_color_and_size(aInput, tCommunicator);
    const auto tObjectiveInputBlocks =
        utilities::group_split_vector(aInput.rawInput(), tGroupColor, utilities::SizeNamedType{tSplitSize});
    const auto tSplitInitialValues =
        utilities::group_split_vector(aInitialValues, tGroupColor, utilities::SizeNamedType{tSplitSize});
    return make_normalized_parallel_aggregate_impl(tObjectiveInputBlocks, AggregateComm{tCommunicator},
                                                   ObjectiveComm{mpi_group(aInput, tCommunicator)},
                                                   tSplitInitialValues);
}
}  // namespace

auto group_split_active_objectives_and_comm(const ValidatedObjectives& aInput)
    -> std::pair<std::vector<ObjectiveFunction>, AggregateComm>
{
    const auto tCommunicator = boost::mpi::communicator{};
    const auto [tGroupColor, tSplitSize] = objective_group_color_and_size(aInput, tCommunicator);
    const auto tObjectiveInputBlocks =
        utilities::group_split_vector(aInput.rawInput(), tGroupColor, utilities::SizeNamedType{tSplitSize});
    return std::make_pair(
        active_parallel_objectives(tObjectiveInputBlocks, ObjectiveComm{mpi_group(aInput, tCommunicator)}),
        AggregateComm{tCommunicator});
}

auto make_parallel_aggregate(const ValidatedObjectives& aInput) -> ParallelAggregateObjective
{
    const auto tCommunicator = boost::mpi::communicator{};
    const auto [tGroupColor, tSplitSize] = objective_group_color_and_size(aInput, tCommunicator);
    const auto tObjectiveInputBlocks =
        utilities::group_split_vector(aInput.rawInput(), tGroupColor, utilities::SizeNamedType{tSplitSize});
    return make_parallel_aggregate_impl(tObjectiveInputBlocks, AggregateComm{tCommunicator},
                                        ObjectiveComm{mpi_group(aInput, tCommunicator)});
}
}  // namespace detail

auto make_aggregate_objective_function(const ValidatedObjectives& aInput) -> ObjectiveFunction
{
    return make_aggregate_function_with_first_derivative(detail::make_parallel_aggregate(aInput));
}

auto make_aggregate_objective_function(const ValidatedObjectives& aInput, const std::vector<double>& aInitialValues)
    -> ObjectiveFunction
{
    return make_aggregate_function_with_first_derivative(
        detail::make_normalized_parallel_aggregate(aInput, aInitialValues));
}

auto number_of_processors_per_objective(const ValidatedObjectives& aInput) -> std::vector<unsigned int>
{
    const auto tGetNumProcs = [](const auto& aObjective) {
        return input_validation::get_input_block<input_parser::objective>(aObjective).number_of_processors.value_or(1U);
    };

    auto tNumberOfProcessors = std::vector<unsigned int>{};
    utilities::transform_if(aInput.rawInput(), std::back_inserter(tNumberOfProcessors), tGetNumProcs,
                            detail::kIsActive);
    return tNumberOfProcessors;
}

bool normalization_by_initial_value_is_needed(const ValidatedObjectives& aValidatedObjectives)
{
    return std::any_of(aValidatedObjectives.rawInput().begin(), aValidatedObjectives.rawInput().end(),
                       detail::kNormalizeByInitialValue);
}

std::vector<double> active_objective_values(const ValidatedObjectives& aInput,
                                            const analysis::AnalysisDomainMesh& aGeometry)
{
    const auto [tObjectiveFunctions, tCommFromSplit] = detail::group_split_active_objectives_and_comm(aInput);
    const auto tObjectiveComm = detail::mpi_group(aInput, tCommFromSplit.mValue);

    std::vector<double> tValues;
    for (const auto& tFunction : tObjectiveFunctions)
    {
        const auto tValue = tFunction.template evaluate<0>(aGeometry);
        plato::utilities::execute_on_root(tObjectiveComm, [&tValues, tValue]() { tValues.push_back(tValue); });
    }
    return plato::utilities::concatenate_over_all_ranks(tValues, tCommFromSplit.mValue);
}

}  // namespace plato::criteria::library
