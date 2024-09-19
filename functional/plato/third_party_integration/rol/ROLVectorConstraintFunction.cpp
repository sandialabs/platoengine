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
    std::cout << "Vector constraint apply jacobian. " << std::endl;
    std::cout << "Size of direction: " << aDirection.size() << std::endl;
    std::cout << "Size of control: " << aControl.size() << std::endl;

    aJacobianTimesDirection = mFunctionWithDfAsJacobian.df(linear_algebra::DynamicVector<double>(aControl))
                                  .mJacobianTimesVectorFunction(linear_algebra::DynamicVector<double>(aDirection))
                                  .stdVector();
}

void ROLVectorConstraintFunction::applyAdjointJacobian(std::vector<double>& aAdjointJacobianTimesDirection,
                                                       const std::vector<double>& aDual,
                                                       const std::vector<double>& aControl,
                                                       double& /*aTolerance*/)
{
    std::cout << "Vector constraint apply adjoint jacobian. " << std::endl;
    std::cout << "Size of dual: " << aDual.size() << std::endl;
    std::cout << "Size of control: " << aControl.size() << std::endl;

    assert(aDual.dimension() == 1);
    assert(aAdjointJacobianTimesDirection.dimension() == aControl.dimension());

    const auto tTemp = mFunctionWithDfAsAdjointJacobian.df(linear_algebra::DynamicVector<double>(aControl))
                           .mJacobianTimesVectorFunction(linear_algebra::DynamicVector<double>(aDual))
                           .stdVector();
    std::cout << "Temp size " << tTemp.size() << std::endl;

    aAdjointJacobianTimesDirection =
        mFunctionWithDfAsAdjointJacobian.df(linear_algebra::DynamicVector<double>(aControl))
            .mJacobianTimesVectorFunction(linear_algebra::DynamicVector<double>(aDual))
            .stdVector();
}
/*
void ROLVectorConstraintFunction::applyAdjointHessian(std::vector<double>& ,
                                                      const std::vector<double>& ,
                                                      const std::vector<double>& ,
                                                      const std::vector<double>& ,
                                                      double& )
{
    std::cout << "A zero apply adjoint hessian call!" << std::endl;
}
*/
}  // namespace plato::third_party_integration::rol
