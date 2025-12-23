#ifndef PLATO_CRITERION_LIBRARY_OBJECTIVEFACTORY
#define PLATO_CRITERION_LIBRARY_OBJECTIVEFACTORY

#include "plato/core/Function.hpp"
#include "plato/core/ParallelAggregate.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::criteria::library
{
using ValidatedObjectives = input_validation::ValidatedComponentType<components::ComponentType::kObjective>;

using ObjectiveEvaluationInfo = core::FunctionInfo<double, core::evaluation::kFunction>;
using ObjectiveGradientInfo =
    core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFirstDerivative>;

using ObjectiveFunction =
    core::Function<const analysis::AnalysisDomainMesh&, ObjectiveEvaluationInfo, ObjectiveGradientInfo>;
using ParallelAggregateObjective =
    core::ParallelAggregate<const analysis::AnalysisDomainMesh&, ObjectiveEvaluationInfo, ObjectiveGradientInfo>;

/// @brief Creates a single aggregate objective function from the objectives defined in @a aInput.
///
/// Evaluates objectives that are to be scaled by initial value using the geometry @a aGeometry and uses the resulting
/// value to scale the weight used for aggregation.
///
/// The objective function is the weighted sum of all objectives defined in @a aInput.
/// There are two modes of parallelization: Fully parallel with parallel objectives, or serial objectives run in
/// serial or parallel. If any parallel objectives are used, then plato must be run with enough ranks to run
/// all objectives in parallel. For example, if there are three objectives requesting 1, 2, and 3 ranks respectively,
/// then plato must be run with 6 ranks. If all objectives are serial, then any number of ranks may be used and
/// objectives will be run in parallel as appropriate.
[[nodiscard]] auto make_aggregate_objective_function(const ValidatedObjectives& aInput,
                                                     const analysis::AnalysisDomainMesh& aGeometry)
    -> ObjectiveFunction;

/// @brief Returns the number of processors required for each objective.
/// @post The size of the returned vector is equal to the number of active objectives in @a aInput.
/// @post The order of the entries in the returned vector matches the order of the entries in @a aInput.
[[nodiscard]] auto number_of_processors_per_objective(const ValidatedObjectives& aInput) -> std::vector<unsigned int>;

namespace detail
{
[[nodiscard]] auto make_parallel_aggregate(const ValidatedObjectives& aInput,
                                           const analysis::AnalysisDomainMesh& aGeometry) -> ParallelAggregateObjective;
}  // namespace detail

}  // namespace plato::criteria::library

#endif
