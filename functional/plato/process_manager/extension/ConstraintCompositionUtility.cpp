#include "plato/process_manager/extension/ConstraintCompositionUtility.hpp"

#include "plato/core/Compose.hpp"

namespace plato::process_manager::extension
{
auto compose_geometry_with_constraint(
    const criteria::library::Constraint<const mesh::MeshDesignVariables&>& aMeshConstraint,
    const plato::geometry::library::FactoryTypes& aGeometry)
    -> criteria::library::Constraint<const linear_algebra::DynamicVector<double>&>
{
    const auto tComposition = compose(aMeshConstraint.mConstraintFunction, aGeometry.mCompute);

    return criteria::library::Constraint<const linear_algebra::DynamicVector<double>&>{
        aMeshConstraint.mName, tComposition, aMeshConstraint.mConstraintTarget, aMeshConstraint.mLinear,
        aMeshConstraint.mConstraintType};
}
}  // namespace plato::process_manager::extension
