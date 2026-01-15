#ifndef PLATO_CRITERION_LIBRARY_OBJECTIVEFACTORY
#define PLATO_CRITERION_LIBRARY_OBJECTIVEFACTORY

#include <format>
#include <ranges>

#include "plato/core/Function.hpp"
#include "plato/core/ParallelAggregate.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/services/SystemLogger.hpp"

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
[[nodiscard]] auto make_aggregate_objective_function(
    const ValidatedObjectives& aInput, const analysis::AnalysisDomainMesh& aGeometry) -> ObjectiveFunction;

/// @brief Returns the number of processors required for each objective.
/// @post The size of the returned vector is equal to the number of active objectives in @a aInput.
/// @post The order of the entries in the returned vector matches the order of the entries in @a aInput.
[[nodiscard]] auto number_of_processors_per_objective(const ValidatedObjectives& aInput) -> std::vector<unsigned int>;

namespace detail
{
/// @brief Holds all aggregation data needed to compute total weight.
struct AggregationData
{
    std::string mName;
    double mWeight = 1.0;
    std::optional<double> mNormalization = std::nullopt;
    double mGoalScaling = 1.0;
};

[[nodiscard]] auto make_parallel_aggregate(const ValidatedObjectives& aInput,
                                           const analysis::AnalysisDomainMesh& aGeometry) -> ParallelAggregateObjective;

/// @brief Computes the aggregation weight based on scaling, user-defined weight, and normalization.
[[nodiscard]] auto total_weight(const AggregationData& aAggregationData) -> double;

/// @brief Logs a summary of the aggregation weights to the console.
void log_aggregate_data(const std::ranges::range auto& aLogData, const boost::mpi::communicator& aAggregatorComm);

void log_aggregate_data(const std::ranges::range auto& aLogData, const boost::mpi::communicator& aAggregatorComm)
{
    aAggregatorComm.barrier();

    auto tWorldLogger = services::component_logger(components::ComponentType::kObjective, "aggregator");
    tWorldLogger.logInfo("Aggregation weight = goal scaling * weight / normalization");

    aAggregatorComm.barrier();

    for (const auto& tLogData : aLogData)
    {
        auto tLogger = services::component_logger(components::ComponentType::kObjective, tLogData.mName,
                                                  boost::mpi::communicator{MPI_COMM_SELF, boost::mpi::comm_attach});
        tLogger.logInfo(std::format("Aggregation weight: {: 8.7e} = {:2} * {} / {}", total_weight(tLogData),
                                    tLogData.mGoalScaling, tLogData.mWeight, tLogData.mNormalization.value_or(1.0)));
    }
    aAggregatorComm.barrier();
}

}  // namespace detail

}  // namespace plato::criteria::library

#endif
