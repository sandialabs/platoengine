#include "plato/process_manager/extension/ROLUtilities.hpp"

#include <ROL_Bounds.hpp>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/core/Compose.hpp"
#include "plato/criteria/library/ConstraintAdapter.hpp"
#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/process_manager/extension/ConstraintCompositionUtility.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/third_party_integration/rol/ROLConstraint.hpp"
#include "plato/third_party_integration/rol/ROLConstraintFunction.hpp"
#include "plato/third_party_integration/rol/Utilities.hpp"

namespace plato::process_manager::extension
{

auto make_rol_objective(const library::ProcessManagerData& aProblem)
    -> std::unique_ptr<plato::third_party_integration::rol::ROLObjectiveFunction>
{
    return std::make_unique<plato::third_party_integration::rol::ROLObjectiveFunction>(
        compose(aProblem.mObjective, aProblem.mGeometry.mCompute));
}

auto make_rol_constraints(const library::ProcessManagerData& aProblem)
    -> std::vector<third_party_integration::rol::ROLConstraint>
{
    std::vector<third_party_integration::rol::ROLConstraint> tROLConstraints;
    std::transform(aProblem.mConstraints.cbegin(), aProblem.mConstraints.cend(), std::back_inserter(tROLConstraints),
                   [&aProblem](const auto& aConstraintData)
                   {
                       const auto tConstraint = compose_geometry_with_constraint(aConstraintData, aProblem.mGeometry);
                       const auto tVectorConstraint = criteria::library::make_vector_constraint(tConstraint);
                       /// TODO add sizing info
                       const auto tConstraintSize = static_cast<unsigned int>(
                           tVectorConstraint.mFunctionWithDfAsJacobian.f(aProblem.mGeometry.mInitialGuess).size());
                       std::cout << "Constraint size determined: " << tConstraintSize << std::endl;

                       return third_party_integration::rol::ROLConstraint{
                           aConstraintData.mName, tConstraintSize, aConstraintData.mLinear,
                           aConstraintData.mConstraintType,
                           std::make_unique<third_party_integration::rol::ROLVectorConstraintFunction>(
                               std::move(tVectorConstraint))};
                   });
    return tROLConstraints;
}

auto make_rol_problem(const library::ProcessManagerData& aProblem)
    -> std::pair<ROL::Ptr<ROL::Problem<double>>, ROL::Ptr<ROL::StdVector<double>>>
{
    namespace tpir = third_party_integration::rol;
    auto tControls = tpir::make_rol_vector(aProblem.mGeometry.mInitialGuess);
    auto tROLProblem = ROL::makePtr<ROL::Problem<double>>(
        ROL::Ptr<ROL::StdObjective<double>>(make_rol_objective(aProblem).release()), tControls);
    tROLProblem->addBoundConstraint(tpir::create_rol_bound_constraint(aProblem.mGeometry.mBounds));
    for (auto& tConstraint : make_rol_constraints(aProblem))
    {
        add_constraint_to_problem(*tROLProblem, tConstraint);
    }
    ///@todo Determine how ROL lumps constraints - should this only be false if they are all linear constraints?
    constexpr bool tLumpConstraints = false;
    constexpr bool tPrintToStream = true;
    tROLProblem->finalize(tLumpConstraints, tPrintToStream, std::cout);
    return {tROLProblem, tControls};
}

auto make_rol_problem_without_constraints_for_gradient_check(const library::ProcessManagerData& aProblem)
    -> std::pair<ROL::Ptr<ROL::Problem<double>>, ROL::Ptr<ROL::StdVector<double>>>
{
    auto tProblem = aProblem;
    tProblem.mConstraints.clear();
    return make_rol_problem(tProblem);
}

}  // namespace plato::process_manager::extension
