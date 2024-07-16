#include "plato/process_manager/extension/ROLUtilities.hpp"

#include <ROL_Bounds.hpp>

#include "plato/core/Compose.hpp"
#include "plato/core/MeshProxy.hpp"
#include "plato/criteria/extension/NodalSumObjective.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/third_party_integration/rol/Utilities.hpp"

namespace plato::process_manager::extension
{
namespace
{
[[nodiscard]] ROL::Ptr<ROL::Bounds<double>> make_rol_bound_constraint(
    const std::pair<std::vector<double>, std::vector<double>>& aBounds)
{
    auto tLowerBounds = ROL::makePtr<std::vector<double>>(aBounds.first);
    auto tUpperBounds = ROL::makePtr<std::vector<double>>(aBounds.second);
    auto tLowerBoundsVector = ROL::makePtr<ROL::StdVector<double>>(std::move(tLowerBounds));
    auto tUpperBoundsVector = ROL::makePtr<ROL::StdVector<double>>(std::move(tUpperBounds));
    return ROL::makePtr<ROL::Bounds<double>>(std::move(tLowerBoundsVector), std::move(tUpperBoundsVector));
}
}  // namespace

std::unique_ptr<plato::third_party_integration::rol::ROLObjectiveFunction> make_rol_objective(
    const library::ProcessManagerData& aProblem)
{
    return std::make_unique<plato::third_party_integration::rol::ROLObjectiveFunction>(
        compose(aProblem.mObjective, aProblem.mGeometry.mCompute));
}

std::vector<std::unique_ptr<plato::third_party_integration::rol::ROLConstraintFunction>> make_rol_constraints(
    const library::ProcessManagerData& aProblem)
{
    std::vector<std::unique_ptr<plato::third_party_integration::rol::ROLConstraintFunction>> tROLConstraints;
    std::transform(
        aProblem.mConstraints.cbegin(), aProblem.mConstraints.cend(), std::back_inserter(tROLConstraints),
        [&aProblem](const plato::criteria::library::Constraint<const core::MeshProxy&>& aConstraintData)
        {
            plato::criteria::library::Constraint<const linear_algebra::DynamicVector<double>&> tConstraint{
                aConstraintData.mName, compose(aConstraintData.mConstraintFunction, aProblem.mGeometry.mCompute),
                aConstraintData.mConstraintTarget, aConstraintData.mLinear};
            return std::make_unique<plato::third_party_integration::rol::ROLConstraintFunction>(std::move(tConstraint));
        });
    return tROLConstraints;
}

std::unique_ptr<ROL::Problem<double>> make_rol_problem(const library::ProcessManagerData& aProblem)
{
    auto tROLProblem = std::make_unique<ROL::Problem<double>>(
        ROL::Ptr<ROL::Objective<double>>(make_rol_objective(aProblem).release()),
        third_party_integration::rol::make_rol_vector(aProblem.mGeometry.mInitialGuess));
    tROLProblem->addBoundConstraint(make_rol_bound_constraint(aProblem.mGeometry.mBounds));
    for (auto& tConstraint : make_rol_constraints(aProblem))
    {
        const std::string& tName = tConstraint->name();
        if (tConstraint->linear())
        {
            tROLProblem->addLinearConstraint(
                tName, Teuchos::rcp(tConstraint.release()),
                third_party_integration::rol::make_rol_vector(criteria::library::make_dual_vector()));
        }
        else
        {
            tROLProblem->addConstraint(
                tName, Teuchos::rcp(tConstraint.release()),
                third_party_integration::rol::make_rol_vector(criteria::library::make_dual_vector()));
        }
    }
    ///@todo Determine how ROL lumps constraints - should this only be false if they are all linear constraints?
    constexpr bool tLumpConstraints =
        false;  //( mAlgorithmType == input_parser::third_party_integration::rol::algorithm_t::ROL_LINEAR_CONSTRAINT ?
                // false : true
                //);
    constexpr bool tPrintToStream = true;
    tROLProblem->finalize(tLumpConstraints, tPrintToStream, std::cout);
    return tROLProblem;
}

}  // namespace plato::process_manager::extension
