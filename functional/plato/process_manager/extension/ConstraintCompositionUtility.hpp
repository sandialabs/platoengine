#ifndef PLATO_PROCESSMANAGER_EXTENSION_CONSTRAINTCOMPOSITIONUTILITY
#define PLATO_PROCESSMANAGER_EXTENSION_CONSTRAINTCOMPOSITIONUTILITY

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/geometry/library/GeometryFactory.hpp"

namespace plato::process_manager::extension
{

///@brief Take a Constraint<MeshDesignVariables> and compose it with a geometry to create a
/// Constraint<DynamicVector>
[[nodiscard]] auto compose_geometry_with_constraint(
    const criteria::library::Constraint<const analysis::AnalysisDomainMesh&>& aMeshConstraint,
    const plato::geometry::library::FactoryTypes& aGeometry)
    -> criteria::library::Constraint<const linear_algebra::DynamicVector<double>&>;

///@brief Take a VectorConstraint<MeshDesignVariables> and compose it with a geometry to create a
/// VectorConstraint<DynamicVector>
[[nodiscard]] auto compose_geometry_with_vector_constraint(
    const criteria::library::VectorConstraint<const analysis::AnalysisDomainMesh&>& aMeshConstraint,
    const plato::geometry::library::FactoryTypes& aGeometry)
    -> criteria::library::VectorConstraint<const linear_algebra::DynamicVector<double>&>;

}  // namespace plato::process_manager::extension

#endif
