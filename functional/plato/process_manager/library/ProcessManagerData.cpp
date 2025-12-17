#include "plato/process_manager/library/ProcessManagerData.hpp"

#include "plato/geometry/library/GeometryFactory.hpp"

namespace plato::process_manager::library
{
namespace
{
auto make_aggregate_objective(const plato::criteria::library::ValidatedObjectives& aObjectiveInput,
                              const plato::geometry::library::FactoryTypes& aGeometry)
    -> plato::criteria::library::ObjectiveFunction
{
    if (plato::criteria::library::normalization_by_initial_value_is_needed(aObjectiveInput))
    {
        const auto tInitialValues = criteria::library::active_objective_values(
            aObjectiveInput, aGeometry.mCompute.evaluate<core::evaluation::kFunction>(aGeometry.mInitialGuess));
        return plato::criteria::library::make_aggregate_objective_function(aObjectiveInput, tInitialValues);
    }
    else
    {
        return plato::criteria::library::make_aggregate_objective_function(aObjectiveInput);
    }
}
}  // namespace

auto make_process_manager_data(const input_validation::ValidatedInput& aData) -> ProcessManagerData
{
    return ProcessManagerData{
        .mGeometry = plato::geometry::library::make_geometry_data(aData.get<components::ComponentType::kGeometry>()),
        .mObjective = make_aggregate_objective(
            aData.get<components::ComponentType::kObjective>(),
            plato::geometry::library::make_geometry_data(aData.get<components::ComponentType::kGeometry>())),
        .mConstraints =
            plato::criteria::library::make_constraints(aData.get<components::ComponentType::kConstraint>())};
}

}  // namespace plato::process_manager::library
