#include "plato/third_party_integration/rol/ROLVectorConstraintFunction.hpp"

namespace plato::third_party_integration::rol
{
ROLVectorConstraintFunction::ROLVectorConstraintFunction(
    criteria::library::VectorConstraint<const linear_algebra::DynamicVector<double>&> aConstraint)
    : mFunction(std::move(aConstraint.mConstraintFunction)), mConstraintTarget(aConstraint.mConstraintTarget)
{
}

void ROLVectorConstraintFunction::value(std::vector<double>& aConstraints, const std::vector<double>& aControl, double&)
{
    const auto tConstraintValue =
        mFunction.evaluate<core::evaluation::kFunction>(linear_algebra::DynamicVector<double>(aControl));
    const auto tTargetVector = linear_algebra::DynamicVector<double>(tConstraintValue.size(), mConstraintTarget);
    aConstraints = (tConstraintValue + (-1.0 * tTargetVector)).stdVector();
}

void ROLVectorConstraintFunction::applyJacobian(std::vector<double>& aJacobianTimesDirection,
                                                const std::vector<double>& aDirection,
                                                const std::vector<double>& aControl,
                                                double& /*aTolerance*/)
{
    auto tResult = linear_algebra::DynamicVector<double>(aDirection) *
                   mFunction.evaluate<core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>(
                       linear_algebra::DynamicVector<double>(aControl));

    aJacobianTimesDirection = std::move(tResult).stdVector();
}

void ROLVectorConstraintFunction::applyAdjointJacobian(std::vector<double>& aAdjointJacobianTimesDirection,
                                                       const std::vector<double>& aDual,
                                                       const std::vector<double>& aControl,
                                                       double& /*aTolerance*/)
{
    auto tResult =
        linear_algebra::DynamicVector<double>(aDual) *
        mFunction.evaluate<core::evaluation::kFirstDerivative>(linear_algebra::DynamicVector<double>(aControl));

    aAdjointJacobianTimesDirection = std::move(tResult).stdVector();
}

}  // namespace plato::third_party_integration::rol
