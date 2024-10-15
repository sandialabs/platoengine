#include "plato/third_party_integration/rol/ROLVectorConstraintFunction.hpp"

namespace plato::third_party_integration::rol
{
ROLVectorConstraintFunction::ROLVectorConstraintFunction(
    criteria::library::VectorConstraint<const linear_algebra::DynamicVector<double>&> aConstraint)
    : mFunctionWithDfAsJacobian(std::move(aConstraint.mFunctionWithDfAsJacobian)),
      mFunctionWithDfAsAdjointJacobian(std::move(aConstraint.mFunctionWithDfAsAdjointJacobian)),
      mConstraintTarget(aConstraint.mConstraintTarget)
{
}

void ROLVectorConstraintFunction::value(std::vector<double>& aConstraints, const std::vector<double>& aControl, double&)
{
    const auto tConstraintValue = mFunctionWithDfAsJacobian.f(linear_algebra::DynamicVector<double>(aControl));
    const auto tTargetVector = linear_algebra::DynamicVector<double>(tConstraintValue.size(), mConstraintTarget);
    aConstraints = (tConstraintValue + (-1.0 * tTargetVector)).stdVector();
}

void ROLVectorConstraintFunction::applyJacobian(std::vector<double>& aJacobianTimesDirection,
                                                const std::vector<double>& aDirection,
                                                const std::vector<double>& aControl,
                                                double& /*aTolerance*/)
{
    auto tResult = linear_algebra::DynamicVector<double>(aDirection) *
                   mFunctionWithDfAsAdjointJacobian.df(linear_algebra::DynamicVector<double>(aControl));
    aJacobianTimesDirection = std::move(tResult).stdVector();
}

void ROLVectorConstraintFunction::applyAdjointJacobian(std::vector<double>& aAdjointJacobianTimesDirection,
                                                       const std::vector<double>& aDual,
                                                       const std::vector<double>& aControl,
                                                       double& /*aTolerance*/)
{
    assert(aAdjointJacobianTimesDirection.size() == aControl.size());

    auto tResult = linear_algebra::DynamicVector<double>(aDual) *
                   mFunctionWithDfAsJacobian.df(linear_algebra::DynamicVector<double>(aControl));
    aAdjointJacobianTimesDirection = std::move(tResult).stdVector();
}

}  // namespace plato::third_party_integration::rol
