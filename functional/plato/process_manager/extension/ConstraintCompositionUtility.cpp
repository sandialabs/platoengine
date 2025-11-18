#include "plato/process_manager/extension/ConstraintCompositionUtility.hpp"

#include "plato/core/Compose.hpp"

namespace plato::process_manager::extension
{
auto compose_geometry_with_vector_constraint(
    const criteria::library::VectorConstraint<const analysis::AnalysisDomainMesh&>& aMeshConstraint,
    const plato::geometry::library::FactoryTypes& aGeometry)
    -> criteria::library::VectorConstraint<const linear_algebra::DynamicVector<double>&>
{
    auto tComposedVectorConstraintJacobian = core::compose(aMeshConstraint.mConstraintFunction, aGeometry.mCompute);

    return criteria::library::VectorConstraint<const linear_algebra::DynamicVector<double>&>{
        aMeshConstraint.mName, std::move(tComposedVectorConstraintJacobian), aMeshConstraint.mLinear,
        aMeshConstraint.mConstraintType};
}

}  // namespace plato::process_manager::extension
