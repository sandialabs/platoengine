#include "plato/criteria/library/ObjectiveFactory.hpp"

#include <boost/mpi/communicator.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <iterator>
#include <ranges>

#include "plato/core/ParallelFunction.hpp"
#include "plato/criteria/library/CriterionFactory.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/criteria/library/ObjectiveReciprocal.hpp"
#include "plato/linear_algebra/DynamicVectorSerialization.hpp"
#include "plato/utilities/NamedType.hpp"
#include "plato/utilities/RankSplitVector.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::criteria::library
{
namespace detail
{
namespace
{
using ValidatedObjective = input_validation::ValidatedInputDataBlock<components::ComponentType::kObjective>;
using AggregateComm = utilities::NamedType<boost::mpi::communicator, struct AggregateCommTag>;
using ObjectiveComm = utilities::NamedType<boost::mpi::communicator, struct ObjectiveCommTag>;

[[nodiscard]] bool normalize_by_initial_value(const ValidatedObjective& aObjective)
{
    const auto& tInput = input_validation::get_input_block<input_parser::objective>(aObjective);
    return tInput.normalize_by_initial_value.value_or(false);
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

[[nodiscard]] double normalization_value(const ObjectiveFunction& aFunction,
                                         const ObjectiveComm& aObjectiveComm,
                                         const analysis::AnalysisDomainMesh& aGeometry)

{
    return 1.0 / core::broadcast_from_root(aObjectiveComm.mValue, aFunction.template evaluate<0>(aGeometry));
}

[[nodiscard]] auto group_split_vector(const ValidatedObjectives& aInput, const boost::mpi::communicator& aComm)
{
    const auto tNumberOfProcessors = number_of_processors_per_objective(aInput);
    const auto tGroupColor = utilities::rank_group_color(tNumberOfProcessors, utilities::RankNamedType{aComm.rank()});
    const auto tNumberOfGroups = boost::numeric_cast<int>(tNumberOfProcessors.size());
    const auto tSplitSize = std::min(aComm.size(), tNumberOfGroups);
    return utilities::group_split_vector(aInput.rawInput(), tGroupColor, utilities::SizeNamedType{tSplitSize});
}

[[nodiscard]] auto mpi_group(const ValidatedObjectives& aInput, const boost::mpi::communicator& aComm)
    -> boost::mpi::communicator
{
    const auto tNumberOfProcessors = number_of_processors_per_objective(aInput);
    const auto tGroupColor = utilities::rank_group_color(tNumberOfProcessors, utilities::RankNamedType{aComm.rank()});
    return aComm.split(tGroupColor.mValue);
}

[[nodiscard]] auto make_parallel_aggregate_impl(const std::vector<ValidatedObjective>& aObjectives,
                                                const AggregateComm& aAggregatorComm,
                                                const ObjectiveComm& aObjectiveComm,
                                                const analysis::AnalysisDomainMesh& aGeometry)
    -> ParallelAggregateObjective
{
    const auto tToObjectivesAndWeights =
        aObjectives |
        std::views::transform(
            [&aObjectiveComm, &aGeometry](const auto& aObjectiveInput)
            {
                auto tFunction = make_parallel_criterion_function(aObjectiveInput, aObjectiveComm);
                const auto tInitialNormalization = normalize_by_initial_value(aObjectiveInput)
                                                       ? normalization_value(tFunction, aObjectiveComm, aGeometry)
                                                       : 1.0;
                const auto tWeight = tInitialNormalization *
                                     input_validation::get_input_block<input_parser::objective>(aObjectiveInput)
                                         .aggregation_weight.value() *
                                     objective_goal_scaling(aObjectiveInput);

                return std::make_pair(std::move(tFunction), tWeight);
            });
    auto tFunctionsAndWeights = std::vector(tToObjectivesAndWeights.begin(), tToObjectivesAndWeights.end());
    return ParallelAggregateObjective{std::move(tFunctionsAndWeights), aAggregatorComm.mValue};
}
}  // namespace

auto make_parallel_aggregate(const ValidatedObjectives& aInput,
                             const analysis::AnalysisDomainMesh& aGeometry) -> ParallelAggregateObjective
{
    const auto tCommunicator = boost::mpi::communicator{};
    const auto tObjectives = group_split_vector(aInput, tCommunicator);
    return make_parallel_aggregate_impl(tObjectives, AggregateComm{tCommunicator},
                                        ObjectiveComm{mpi_group(aInput, tCommunicator)}, aGeometry);
}
}  // namespace detail

auto make_aggregate_objective_function(const ValidatedObjectives& aInput, const analysis::AnalysisDomainMesh& aGeometry)
    -> ObjectiveFunction
{
    return make_aggregate_function_with_first_derivative(detail::make_parallel_aggregate(aInput, aGeometry));
}

auto number_of_processors_per_objective(const ValidatedObjectives& aInput) -> std::vector<unsigned int>
{
    const auto tNumberOfProcessors =
        aInput.rawInput() | std::views::transform(
                                [](const auto& aObjective) {
                                    return input_validation::get_input_block<input_parser::objective>(aObjective)
                                        .number_of_processors.value_or(1U);
                                });
    return std::vector(tNumberOfProcessors.begin(), tNumberOfProcessors.end());
}

}  // namespace plato::criteria::library
