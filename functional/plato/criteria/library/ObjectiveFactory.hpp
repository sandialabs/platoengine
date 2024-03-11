#ifndef PLATO_CRITERION_LIBRARY_OBJECTIVEFACTORY
#define PLATO_CRITERION_LIBRARY_OBJECTIVEFACTORY

#include "plato/core/Function.hpp"
#include "plato/core/ParallelAggregate.hpp"
#include "plato/core/ValidatedInputTypeWrapper.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::core
{
struct MeshProxy;
}

namespace plato::criteria::library
{
using ValidatedObjectives =
    core::ValidatedInputTypeWrapper<std::vector<core::ValidatedInputTypeWrapper<input_parser::objective>>>;

using ObjectiveFunction = core::Function<double, linear_algebra::DynamicVector<double>, const core::MeshProxy&>;
using AggregateObjective = core::Aggregate<double, linear_algebra::DynamicVector<double>, const core::MeshProxy&>;
using ParallelAggregateObjective =
    core::ParallelAggregate<double, linear_algebra::DynamicVector<double>, const core::MeshProxy&>;

namespace detail
{
[[nodiscard]] AggregateObjective make_aggregate(const ValidatedObjectives& aInput);
[[nodiscard]] ParallelAggregateObjective make_parallel_aggregate(const ValidatedObjectives& aInput);
}  // namespace detail

[[nodiscard]] ObjectiveFunction make_aggregate_objective_function(const ValidatedObjectives& aInput);

}  // namespace plato::criteria::library

#endif
