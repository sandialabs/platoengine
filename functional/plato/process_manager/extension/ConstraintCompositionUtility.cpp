#include "plato/process_manager/extension/ConstraintCompositionUtility.hpp"

#include "plato/core/Compose.hpp"
#include "plato/linear_algebra/ComposeAdjointJacobian.hpp"

namespace plato::process_manager::extension
{
auto compose_geometry_with_constraint(
    const criteria::library::Constraint<const analysis::AnalysisDomainMesh&>& aMeshConstraint,
    const plato::geometry::library::FactoryTypes& aGeometry)
    -> criteria::library::Constraint<const linear_algebra::DynamicVector<double>&>
{
    const auto tComposition = compose(aMeshConstraint.mConstraintFunction, aGeometry.mCompute);

    return criteria::library::Constraint<const linear_algebra::DynamicVector<double>&>{
        aMeshConstraint.mName, tComposition, aMeshConstraint.mConstraintTarget, aMeshConstraint.mLinear,
        aMeshConstraint.mConstraintType};
}

auto compose_geometry_with_vector_constraint(
    const criteria::library::VectorConstraint<const analysis::AnalysisDomainMesh&>& aMeshConstraint,
    const plato::geometry::library::FactoryTypes& aGeometry)
    -> criteria::library::VectorConstraint<const linear_algebra::DynamicVector<double>&>
{
    std::cout << "Precompose" << std::endl;
    const auto tComposedVectorConstraintJacobian =
        core::compose(aMeshConstraint.mFunctionWithDfAsJacobian, aGeometry.mCompute);
    std::cout << "Decompose" << std::endl;
    const auto tComposedVectorConstraintAdjointJacobian =
        linear_algebra::compose_adjoint_jacobian(aMeshConstraint.mFunctionWithDfAsAdjointJacobian, aGeometry.mCompute);

    return criteria::library::VectorConstraint<const linear_algebra::DynamicVector<double>&>{
        aMeshConstraint.mName,
        tComposedVectorConstraintJacobian,
        tComposedVectorConstraintAdjointJacobian,
        aMeshConstraint.mConstraintTarget,
        aMeshConstraint.mLinear,
        aMeshConstraint.mConstraintType};
}

}  // namespace plato::process_manager::extension
