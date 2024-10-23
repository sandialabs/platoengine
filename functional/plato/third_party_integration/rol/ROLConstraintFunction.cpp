#include "plato/third_party_integration/rol/ROLConstraintFunction.hpp"

namespace plato::third_party_integration::rol
{
ROLConstraintFunction::ROLConstraintFunction(
    criteria::library::Constraint<const linear_algebra::DynamicVector<double>&> aConstraint)
    : mFunction(std::move(aConstraint.mConstraintFunction)), mConstraintTarget(aConstraint.mConstraintTarget)
{
}

void ROLConstraintFunction::value(std::vector<double>& aConstraints, const std::vector<double>& aControl, double&)
{
    const double tConstraintValue =
        mFunction.evaluate<core::evaluation::kFunction>(linear_algebra::DynamicVector<double>(aControl));
    aConstraints.front() = tConstraintValue - mConstraintTarget;
}

void ROLConstraintFunction::applyJacobian(std::vector<double>& aJacobianTimesDirection,
                                          const std::vector<double>& aDirection,
                                          const std::vector<double>& aControl,
                                          double& /*aTolerance*/)
{
    aJacobianTimesDirection.front() =
        mFunction.evaluate<core::evaluation::kFirstDerivative>(linear_algebra::DynamicVector<double>(aControl))
            .dot(linear_algebra::DynamicVector<double>(aDirection));
}

void ROLConstraintFunction::applyAdjointJacobian(std::vector<double>& aAdjointJacobianTimesDirection,
                                                 const std::vector<double>& aDual,     // NOLINT
                                                 const std::vector<double>& aControl,  // NOLINT
                                                 double& /*aTolerance*/)
{
    assert(aDual.size() == 1);
    assert(aAdjointJacobianTimesDirection.size() == aControl.size());
    aAdjointJacobianTimesDirection =
        (mFunction.evaluate<core::evaluation::kFirstDerivative>(linear_algebra::DynamicVector<double>(aControl)) *
         aDual.front())
            .stdVector();
}

void ROLConstraintFunction::applyAdjointHessian(std::vector<double>& aHessianUV,
                                                const std::vector<double>& /*u*/,
                                                const std::vector<double>& /*v*/,
                                                const std::vector<double>& /*x*/,
                                                double& /*tol*/)
{
    aHessianUV = std::vector<double>(aHessianUV.size(), 0.0);
}

}  // namespace plato::third_party_integration::rol
