#include "plato/process_manager/library/ProcessManagerData.hpp"

#include "plato/geometry/library/GeometryFactory.hpp"

namespace plato::process_manager::library
{
auto make_process_manager_data(const input_validation::ValidatedInput& aData) -> ProcessManagerData
{
    return ProcessManagerData{
        plato::geometry::library::make_geometry_data(aData.get<components::ComponentType::kGeometry>()),
        plato::criteria::library::make_aggregate_objective_function(aData.get<components::ComponentType::kObjective>()),
        plato::criteria::library::make_constraints(aData.get<components::ComponentType::kConstraint>())};
}

}  // namespace plato::process_manager::library
