#include "plato/process_manager/library/ProcessManagerData.hpp"

namespace plato::process_manager::library
{
auto make_process_manager_data(const input_validation::ValidatedInput& aData) -> ProcessManagerData
{
    return ProcessManagerData{
        plato::geometry::library::make_geometry_data(aData.get<input_parser::ComponentType::kGeometry>()),
        plato::criteria::library::make_aggregate_objective_function(
            aData.get<input_parser::ComponentType::kObjective>()),
        plato::criteria::library::make_constraints(aData.get<input_parser::ComponentType::kConstraint>())};
}

}  // namespace plato::process_manager::library
