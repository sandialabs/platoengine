#include "plato/criteria/library/ObjectiveFactory.hpp"

#include <boost/mpi/communicator.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <iterator>
#include <numeric>

#include "plato/core/ParallelFunction.hpp"
#include "plato/core/ValidatedInputTypeWrapper.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/criteria/library/CriterionFactory.hpp"
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
using AggregateComm = utilities::NamedType<boost::mpi::communicator, struct AggregateCommTag>;
using ObjectiveComm = utilities::NamedType<boost::mpi::communicator, struct ObjectiveCommTag>;

const auto kIsActive = [](const auto& aObjective) { return core::is_active(aObjective.rawInput()); };

bool is_parallel_objective(const input_parser::objective& aObjective)
{
    return aObjective.number_of_processors.value_or(1u) > 1u;
}

auto make_parallel_criterion_function(const core::ValidatedInputTypeWrapper<input_parser::objective>& aObjective,
                                      const ObjectiveComm& aObjectiveComm)
{
    if (is_parallel_objective(aObjective.rawInput()))
    {
        return core::adapt_parallel_function(make_criterion_function(aObjective, aObjectiveComm.mValue),
                                             aObjectiveComm.mValue);
    }
    else
    {
        return make_criterion_function(aObjective);
    }
}

ParallelAggregateObjective make_parallel_aggregate_impl(
    const std::vector<core::ValidatedInputTypeWrapper<input_parser::objective>>& tObjectives,
    const AggregateComm& aAggregatorComm,
    const ObjectiveComm& aObjectiveComm)
{
    using ObjectiveAndWeight = std::pair<ObjectiveFunction, double>;
    std::vector<ObjectiveAndWeight> tFunctionsAndWeights;
    utilities::transform_if(
        tObjectives, std::back_inserter(tFunctionsAndWeights),
        [&aObjectiveComm](const auto& aObjective)
        {
            const double tWeight = aObjective.rawInput().aggregation_weight.value();
            return std::make_pair(make_parallel_criterion_function(aObjective, aObjectiveComm), tWeight);
        },
        kIsActive);
    return ParallelAggregateObjective{std::move(tFunctionsAndWeights), aAggregatorComm.mValue};
}

auto group_split_vector(const ValidatedObjectives& aInput, const boost::mpi::communicator& aComm)
    -> std::vector<core::ValidatedInputTypeWrapper<input_parser::objective>>
{
    const auto tNumberOfProcessors = number_of_processors_per_objective(aInput);
    const auto tGroupColor = utilities::rank_group_color(tNumberOfProcessors, utilities::RankNamedType{aComm.rank()});
    const auto tNumberOfGroups = boost::numeric_cast<int>(tNumberOfProcessors.size());
    const auto tSplitSize = std::min(aComm.size(), tNumberOfGroups);
    return utilities::group_split_vector(aInput.rawInput(), tGroupColor, utilities::SizeNamedType{tSplitSize});
}

boost::mpi::communicator mpi_group(const ValidatedObjectives& aInput, const boost::mpi::communicator& aComm)
{
    const auto tNumberOfProcessors = number_of_processors_per_objective(aInput);
    const auto tGroupColor = utilities::rank_group_color(tNumberOfProcessors, utilities::RankNamedType{aComm.rank()});
    return aComm.split(tGroupColor.mValue);
}
}  // namespace

ParallelAggregateObjective make_parallel_aggregate(const ValidatedObjectives& aInput)
{
    const auto tCommunicator = boost::mpi::communicator{};
    const auto tObjectives = group_split_vector(aInput, tCommunicator);
    return make_parallel_aggregate_impl(tObjectives, AggregateComm{tCommunicator},
                                        ObjectiveComm{mpi_group(aInput, tCommunicator)});
}

}  // namespace detail

ObjectiveFunction make_aggregate_objective_function(const ValidatedObjectives& aInput)
{
    return make_aggregate_function(detail::make_parallel_aggregate(aInput));
}

std::vector<unsigned int> number_of_processors_per_objective(const ValidatedObjectives& aInput)
{
    const auto tGetNumProcs = [](const auto& aObjective)
    { return aObjective.rawInput().number_of_processors.value_or(1u); };
    auto tNumberOfProcessors = std::vector<unsigned int>{};
    utilities::transform_if(aInput.rawInput(), std::back_inserter(tNumberOfProcessors), tGetNumProcs,
                            detail::kIsActive);
    return tNumberOfProcessors;
}

}  // namespace plato::criteria::library
