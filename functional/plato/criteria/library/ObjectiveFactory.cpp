#include "plato/criteria/library/ObjectiveFactory.hpp"

#include <boost/mpi/communicator.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <iterator>
#include <numeric>

#include "plato/core/ParallelFunction.hpp"
#include "plato/criteria/library/CriterionFactory.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/criteria/library/ObjectiveModifications.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
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

const auto kIsActive = [](const auto& aObjective)
{ return input_validation::is_active(input_validation::get_input_block<input_parser::objective>(aObjective)); };

[[nodiscard]] auto get_objective_goal(const ValidatedObjective& aObjective) -> ObjectiveGoal
{
    return input_validation::get_input_block<input_parser::objective>(aObjective)
        .objective_goal.value_or(ObjectiveGoal::kMinimize);
}

[[nodiscard]] auto objective_goal_scaling(const ValidatedObjective& aObjective) -> double
{
    return get_objective_goal(aObjective) == ObjectiveGoal::kMinimizeNegation ? -1.0 : 1.0;
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
                make_criterion_function<CriterionFunction, input_parser::objective>(aObjective, aObjectiveComm.mValue),
                get_objective_goal(aObjective)),
            aObjectiveComm.mValue);
    }
    else
    {
        return make_reciprocal_criterion_function(
            make_criterion_function<CriterionFunction, input_parser::objective>(aObjective),
            get_objective_goal(aObjective));
    }
}

[[nodiscard]] auto make_parallel_aggregate_impl(const std::vector<ValidatedObjective>& aObjectives,
                                                const AggregateComm& aAggregatorComm,
                                                const ObjectiveComm& aObjectiveComm) -> ParallelAggregateObjective
{
    using ObjectiveAndWeight = std::pair<ObjectiveFunction, double>;
    std::vector<ObjectiveAndWeight> tFunctionsAndWeights;
    utilities::transform_if(
        aObjectives, std::back_inserter(tFunctionsAndWeights),
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

[[nodiscard]] auto group_split_vector(const ValidatedObjectives& aInput, const boost::mpi::communicator& aComm)
{
    const auto tNumberOfProcessors = number_of_processors_per_objective(aInput);
    const auto tGroupColor = utilities::rank_group_color(tNumberOfProcessors, utilities::RankNamedType{aComm.rank()});
    const auto tNumberOfGroups = boost::numeric_cast<int>(tNumberOfProcessors.size());
    const auto tSplitSize = std::min(aComm.size(), tNumberOfGroups);
    return utilities::group_split_vector(aInput.rawInput(), tGroupColor, utilities::SizeNamedType{tSplitSize});
}

[[nodiscard]] auto mpi_group(const ValidatedObjectives& aInput,
                             const boost::mpi::communicator& aComm) -> boost::mpi::communicator
{
    const auto tNumberOfProcessors = number_of_processors_per_objective(aInput);
    const auto tGroupColor = utilities::rank_group_color(tNumberOfProcessors, utilities::RankNamedType{aComm.rank()});
    return aComm.split(tGroupColor.mValue);
}

}  // namespace

auto make_parallel_aggregate(const ValidatedObjectives& aInput) -> ParallelAggregateObjective
{
    const auto tCommunicator = boost::mpi::communicator{};
    const auto tObjectives = group_split_vector(aInput, tCommunicator);
    return make_parallel_aggregate_impl(tObjectives, AggregateComm{tCommunicator},
                                        ObjectiveComm{mpi_group(aInput, tCommunicator)});
}

}  // namespace detail

auto make_aggregate_objective_function(const ValidatedObjectives& aInput) -> ObjectiveFunction
{
    return make_aggregate_function_with_first_derivative(detail::make_parallel_aggregate(aInput));
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

}  // namespace plato::criteria::library
