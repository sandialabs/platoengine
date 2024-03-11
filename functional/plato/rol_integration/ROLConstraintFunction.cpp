#include "plato/rol_integration/ROLConstraintFunction.hpp"

#include "plato/rol_integration/ROLHelpers.hpp"

namespace plato::rol_integration
{
ROLConstraintFunction::ROLConstraintFunction(
    criteria::library::Constraint<const linear_algebra::DynamicVector<double>&> aConstraint)
    : mName(aConstraint.mName),
      mFunction(std::move(aConstraint.mConstraintFunction)),
      mConstraintTarget(aConstraint.mConstraintTarget),
      mLinear(aConstraint.mLinear)
{
}

void ROLConstraintFunction::value(ROL::Vector<double>& aConstraints, const ROL::Vector<double>& aControl, double&)
{
    const double tConstraintValue = mFunction.f(to_dynamic_vector(aControl));

    const double tOutput = tConstraintValue - mConstraintTarget;
    auto& aConstraintsAsStdVector = dynamic_cast<ROL::StdVector<double>&>(aConstraints);
    aConstraintsAsStdVector.getVector()->front() = tOutput;
}

void ROLConstraintFunction::applyJacobian(ROL::Vector<double>& aJacobianTimesDirection,
                                          const ROL::Vector<double>& aDirection,
                                          const ROL::Vector<double>& aControl,
                                          double& /*aTolerance*/)
{
    const double tJacobianTimesDirection = mFunction.df(to_dynamic_vector(aControl)).dot(to_dynamic_vector(aDirection));
    assign_vector(aJacobianTimesDirection, {tJacobianTimesDirection});
}

void ROLConstraintFunction::applyAdjointJacobian(ROL::Vector<double>& aAdjointJacobianTimesDirection,
                                                 const ROL::Vector<double>& aDual,
                                                 const ROL::Vector<double>& aControl,
                                                 double& /*aTolerance*/)
{
    assert(aDual.dimension() == 1);
    assert(aAdjointJacobianTimesDirection.dimension() == aControl.dimension());
    auto tAdjointJacobianTimesDirection = mFunction.df(to_dynamic_vector(aControl));
    auto tAdjointJacobianTimesDirectionROLVector = to_rol_vector(tAdjointJacobianTimesDirection);

    const auto& tDualAsStdVector = dynamic_cast<const ROL::StdVector<double>&>(aDual);
    tAdjointJacobianTimesDirectionROLVector.scale(tDualAsStdVector.getVector()->front());
    auto tAdjointJacobianTimesDirectionAsStdVector =
        dynamic_cast<const ROL::StdVector<double>&>(aAdjointJacobianTimesDirection);
    tAdjointJacobianTimesDirectionAsStdVector.set(tAdjointJacobianTimesDirectionROLVector);
}

void ROLConstraintFunction::applyAdjointJacobian(ROL::Vector<double>& aAdjointJacobianTimesDirection,
                                                 const ROL::Vector<double>& aDual,
                                                 const ROL::Vector<double>& aControl,
                                                 const ROL::Vector<double>& /*aDualV*/,
                                                 double& aTolerance)
{
    applyAdjointJacobian(aAdjointJacobianTimesDirection, aDual, aControl, aTolerance);
}

bool ROLConstraintFunction::linear() const { return mLinear; }

const std::string& ROLConstraintFunction::name() const { return mName; }

}  // namespace plato::rol_integration
