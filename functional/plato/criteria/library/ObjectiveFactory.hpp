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
using ParallelAggregateObjective =
    core::ParallelAggregate<double, linear_algebra::DynamicVector<double>, const core::MeshProxy&>;

/// @brief Creates the objectives function from the objectives defined in @a aInput.
///
/// The objective function is the weighted sum of all objectives defined in @a aInput.
/// There are two modes of parallelization: Fully parallel with parallel objectives, or serial objectives run in
/// serial or parallel. If any parallel objectives are used, then plato must be run with enough ranks to run
/// all objectives in parallel. For example, if there are three objectives requesting 1, 2, and 3 ranks respectively,
/// then plato must be run with 6 ranks. If all objectives are serial, then any number of ranks may be used and
/// objectives will be run in parallel as appropriate.
[[nodiscard]] ObjectiveFunction make_aggregate_objective_function(const ValidatedObjectives& aInput);

/// @brief Returns the number of processors required for each objective.
/// @post The size of the returned vector is equal to the number of active objectives in @a aInput.
/// @post The order of the entries in the returned vector matches the order of the entries in @a aInput.
[[nodiscard]] std::vector<unsigned int> number_of_processors_per_objective(const ValidatedObjectives& aInput);

namespace detail
{
[[nodiscard]] ParallelAggregateObjective make_parallel_aggregate(const ValidatedObjectives& aInput);
}  // namespace detail

}  // namespace plato::criteria::library

#endif
