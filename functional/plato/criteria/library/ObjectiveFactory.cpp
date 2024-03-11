#include "plato/criteria/library/ObjectiveFactory.hpp"

#include <boost/mpi/communicator.hpp>

#include "plato/core/ValidatedInputTypeWrapper.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/criteria/library/CriterionFactory.hpp"
#include "plato/linear_algebra/DynamicVectorSerialization.hpp"
#include "plato/utilities/RankSplitVector.hpp"

namespace plato::criteria::library
{
namespace detail
{
namespace
{
template <typename AggregateType, typename... Args>
AggregateType make_aggregate_impl(
    const std::vector<core::ValidatedInputTypeWrapper<input_parser::objective>>& tObjectives, Args&&... aArgs)
{
    using ObjectiveAndWeight = std::pair<ObjectiveFunction, double>;
    std::vector<ObjectiveAndWeight> tFunctionsAndWeights;
    for (const auto& tObjective : tObjectives)
    {
        if (core::is_active(tObjective.rawInput()))
        {
            const double tWeight = tObjective.rawInput().aggregation_weight.value();
            tFunctionsAndWeights.emplace_back(make_criterion_function(tObjective), tWeight);
        }
    }
    return AggregateType{std::move(tFunctionsAndWeights), std::forward<Args>(aArgs)...};
}

auto rank_split_vector(const std::vector<core::ValidatedInputTypeWrapper<input_parser::objective>>& aInputs,
                       const boost::mpi::communicator& aComm)
    -> std::vector<core::ValidatedInputTypeWrapper<input_parser::objective>>
{
    return plato::utilities::rank_split_vector(aInputs,
                                                           plato::utilities::RankNamedType{aComm.rank()},
                                                           plato::utilities::SizeNamedType{aComm.size()});
}
}  // namespace

AggregateObjective make_aggregate(const ValidatedObjectives& aInput)
{
    return make_aggregate_impl<AggregateObjective>(aInput.rawInput());
}

ParallelAggregateObjective make_parallel_aggregate(const ValidatedObjectives& aInput)
{
    const auto tCommunicator = boost::mpi::communicator{};
    const auto tObjectives = rank_split_vector(aInput.rawInput(), tCommunicator);
    return make_aggregate_impl<ParallelAggregateObjective>(tObjectives, tCommunicator);
}

}  // namespace detail

ObjectiveFunction make_aggregate_objective_function(const ValidatedObjectives& aInput)
{
    return make_aggregate_function(detail::make_parallel_aggregate(aInput));
}

}  // namespace plato::criteria::library
