#include "plato/third_party_integration/rol/ROLVectorConstraintFunction.hpp"

namespace plato::third_party_integration::rol
{

namespace
{
void print(const std::vector<double>& aVector)
{
    for (const auto& x : aVector)
    {
        std::cout << x << " ";
    }
    std::cout << std::endl;
}
}  // namespace

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
    std::cout << "Apply jacobian " << std::endl;
    std::cout << "control : ";
    print(aControl);
    std::cout << "direction : ";
    print(aDirection);

    aJacobianTimesDirection = mFunctionWithDfAsJacobian.df(linear_algebra::DynamicVector<double>(aControl))
                                  .mJacobianTimesVectorFunction(linear_algebra::DynamicVector<double>(aDirection))
                                  .stdVector();

    std::cout << "j*v ";
    print(aJacobianTimesDirection);
}

void ROLVectorConstraintFunction::applyAdjointJacobian(std::vector<double>& aAdjointJacobianTimesDirection,
                                                       const std::vector<double>& aDual,
                                                       const std::vector<double>& aControl,
                                                       double& /*aTolerance*/)
{
    assert(aAdjointJacobianTimesDirection.size() == aControl.size());
    std::cout << "Apply adjoint jacobian " << std::endl;
    std::cout << "control : ";
    print(aControl);
    std::cout << "dual : ";
    print(aDual);

    aAdjointJacobianTimesDirection =
        mFunctionWithDfAsAdjointJacobian.df(linear_algebra::DynamicVector<double>(aControl))
            .mJacobianTimesVectorFunction(linear_algebra::DynamicVector<double>(aDual))
            .stdVector();
    std::cout << "j^T*d ";
    print(aAdjointJacobianTimesDirection);
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
