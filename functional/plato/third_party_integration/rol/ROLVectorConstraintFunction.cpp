#include "plato/third_party_integration/rol/ROLVectorConstraintFunction.hpp"

namespace plato::third_party_integration::rol
{
ROLVectorConstraintFunction::ROLVectorConstraintFunction(ROLPlatoFunction aConstraint)
    : mFunction(std::move(aConstraint))
{
}

void ROLVectorConstraintFunction::value(std::vector<double>& aConstraints, const std::vector<double>& aControl, double&)
{
    auto tConstraintValue =
        mFunction.evaluate<core::evaluation::kFunction>(linear_algebra::DynamicVector<double>(aControl));
    aConstraints = std::move(tConstraintValue).stdVector();
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

void ROLVectorConstraintFunction::applyAdjointHessian(std::vector<double>& aHessianUV,
                                                      const std::vector<double>& /*u*/,
                                                      const std::vector<double>& /*v*/,
                                                      const std::vector<double>& /*x*/,
                                                      double& /*tol*/)
{
    aHessianUV = std::vector<double>(aHessianUV.size(), 0.0);
}

}  // namespace plato::third_party_integration::rol
