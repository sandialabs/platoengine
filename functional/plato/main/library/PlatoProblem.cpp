#include "plato/main/library/PlatoProblem.hpp"

#include <ROL_Bounds.hpp>

#include "plato/core/Compose.hpp"
#include "plato/core/MeshProxy.hpp"
#include "plato/criteria/extension/NodalSumObjective.hpp"
#include "plato/rol_integration/ROLHelpers.hpp"

namespace plato::main::library
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

PlatoProblem make_plato_problem(const ValidatedInput& aData)
{
    return PlatoProblem{plato::geometry::library::make_geometry_data(aData.geometry()),
                        plato::criteria::library::make_aggregate_objective_function(aData.objectives()),
                        plato::criteria::library::make_constraints(aData.constraints()),
                        plato::optimizer::rol_parameter_list(aData.optimizationParameters())};
}

std::unique_ptr<plato::rol_integration::ROLObjectiveFunction> make_rol_objective(
    const PlatoProblem& aProblem)
{
    return std::make_unique<plato::rol_integration::ROLObjectiveFunction>(
        compose(aProblem.mObjective, aProblem.mGeometry.mCompute));
}

std::vector<std::unique_ptr<plato::rol_integration::ROLConstraintFunction>> make_rol_constraints(
    const PlatoProblem& aProblem)
{
    std::vector<std::unique_ptr<plato::rol_integration::ROLConstraintFunction>> tROLConstraints;
    std::transform(
        aProblem.mConstraints.cbegin(), aProblem.mConstraints.cend(), std::back_inserter(tROLConstraints),
        [&aProblem](const plato::criteria::library::Constraint<const core::MeshProxy&>& aConstraintData)
        {
            plato::criteria::library::Constraint<const linear_algebra::DynamicVector<double>&> tConstraint{
                aConstraintData.mName, compose(aConstraintData.mConstraintFunction, aProblem.mGeometry.mCompute),
                aConstraintData.mConstraintTarget, aConstraintData.mLinear};
            return std::make_unique<plato::rol_integration::ROLConstraintFunction>(std::move(tConstraint));
        });
    return tROLConstraints;
}

std::unique_ptr<plato::rol_integration::ROLObjectiveFunction> make_rol_sensitivity_objective(
    const PlatoProblem& aProblem)
{
    auto tSimpleObjectiveFunction = plato::criteria::extension::make_nodal_sum_function();
    return std::make_unique<plato::rol_integration::ROLObjectiveFunction>(
        compose(tSimpleObjectiveFunction, aProblem.mGeometry.mCompute));
}

std::unique_ptr<ROL::Problem<double>> make_rol_problem(const PlatoProblem& aProblem)
{
    auto tROLProblem =
        std::make_unique<ROL::Problem<double>>(ROL::Ptr<ROL::Objective<double>>(make_rol_objective(aProblem).release()),
                                               rol_integration::make_rol_vector(aProblem.mGeometry.mInitialGuess));
    tROLProblem->addBoundConstraint(make_rol_bound_constraint(aProblem.mGeometry.mBounds));
    for (auto& tConstraint : make_rol_constraints(aProblem))
    {
        const std::string& tName = tConstraint->name();
        if (tConstraint->linear())
        {
            tROLProblem->addLinearConstraint(tName, Teuchos::rcp(tConstraint.release()),
                                             rol_integration::make_rol_vector(criteria::library::make_dual_vector()));
        }
        else
        {
            tROLProblem->addConstraint(tName, Teuchos::rcp(tConstraint.release()),
                                       rol_integration::make_rol_vector(criteria::library::make_dual_vector()));
        }
    }
    ///@todo Determine how ROL lumps constraints - should this only be false if they are all linear constraints?
    constexpr bool tLumpConstraints =
        false;  //( mAlgorithmType == input_parser::optimizer::algorithm_t::ROL_LINEAR_CONSTRAINT ? false : true );
    tROLProblem->finalize(tLumpConstraints);  //, tPrintToStream, mOutputFile);
    return tROLProblem;
}

}  // namespace plato::main::library
