#ifndef PLATO_PROCESSMANAGER_EXTENSION_CONSTRAINTCOMPOSITIONUTILITY
#define PLATO_PROCESSMANAGER_EXTENSION_CONSTRAINTCOMPOSITIONUTILITY

#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/geometry/library/GeometryFactory.hpp"
#include "plato/mesh/MeshDesignVariables.hpp"

namespace plato::process_manager::extension
{

///@brief Take a Constraint<MeshDesignVariables> and compose it with a geometry to create a
/// Constraint<DynamicVector>
[[nodiscard]] auto compose_geometry_with_constraint(
    const criteria::library::Constraint<const mesh::MeshDesignVariables&>& aMeshConstraint,
    const plato::geometry::library::FactoryTypes& aGeometry)
    -> criteria::library::Constraint<const linear_algebra::DynamicVector<double>&>;

}  // namespace plato::process_manager::extension

#endif
