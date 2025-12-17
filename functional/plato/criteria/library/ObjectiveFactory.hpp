#ifndef PLATO_CRITERION_LIBRARY_OBJECTIVEFACTORY
#define PLATO_CRITERION_LIBRARY_OBJECTIVEFACTORY

#include <utility>
#include <vector>

#include "plato/core/Function.hpp"
#include "plato/core/ParallelAggregate.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/utilities/NamedType.hpp"

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

using AggregateComm = utilities::NamedType<boost::mpi::communicator, struct AggregateCommTag>;

/// @brief Creates the objectives function from the objectives defined in @a aInput.
///
/// The objective function is the weighted sum of all objectives defined in @a aInput.
/// There are two modes of parallelization: Fully parallel with parallel objectives, or serial objectives run in
/// serial or parallel. If any parallel objectives are used, then plato must be run with enough ranks to run
/// all objectives in parallel. For example, if there are three objectives requesting 1, 2, and 3 ranks respectively,
/// then plato must be run with 6 ranks. If all objectives are serial, then any number of ranks may be used and
/// objectives will be run in parallel as appropriate.
[[nodiscard]] auto make_aggregate_objective_function(const ValidatedObjectives& aInput) -> ObjectiveFunction;

/// @brief Overload that takes in a vector of initial values for the criteria @a aInitialValues and uses them to scale
/// the weight used for aggregation so the objectives are normalized by their initial value.
/// @pre The size of @a aInitialValues must match the number of active objectives and the order of the objective values
/// is assumed to match that in @a aInput.
[[nodiscard]] auto make_aggregate_objective_function(const ValidatedObjectives& aInput,
                                                     const std::vector<double>& aInitialValues) -> ObjectiveFunction;

/// @brief Returns the number of processors required for each objective.
/// @post The size of the returned vector is equal to the number of active objectives in @a aInput.
/// @post The order of the entries in the returned vector matches the order of the entries in @a aInput.
[[nodiscard]] auto number_of_processors_per_objective(const ValidatedObjectives& aInput) -> std::vector<unsigned int>;

/// @brief Returns true if any objective input block includes normalize_by_initial_value=true.
[[nodiscard]] bool normalization_by_initial_value_is_needed(const ValidatedObjectives& aValidatedObjectives);

/// @brief Evaluates objectives defined in @a aInput using the geometry @a aGeometry. Returns a vector
/// containing the value of each objective.
[[nodiscard]] std::vector<double> active_objective_values(const ValidatedObjectives& aInput,
                                                          const analysis::AnalysisDomainMesh& aGeometry);

namespace detail
{
/// @brief Returns a `std::pair` containing a `std::vector` of objective functions to be evaluated on a given rank and
/// the comm used to split them.
[[nodiscard]] auto group_split_active_objectives_and_comm(const ValidatedObjectives& aInput)
    -> std::pair<std::vector<ObjectiveFunction>, AggregateComm>;

[[nodiscard]] auto make_parallel_aggregate(const ValidatedObjectives& aInput) -> ParallelAggregateObjective;
}  // namespace detail

}  // namespace plato::criteria::library

#endif
