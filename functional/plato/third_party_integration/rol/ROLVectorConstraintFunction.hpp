#ifndef PLATO_ROL_INTEGRATION_ROLVECTORCONSTRAINTFUNCTION
#define PLATO_ROL_INTEGRATION_ROLVECTORCONSTRAINTFUNCTION

#include <ROL_Bounds.hpp>
#include <ROL_StdConstraint.hpp>
#include <ROL_Vector.hpp>

#include "plato/core/Function.hpp"
#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::third_party_integration::rol
{
class ROLVectorConstraintFunction : public ROL::StdConstraint<double>
{
   public:
    using ROLPlatoFunction = core::Function<linear_algebra::DynamicVector<double>,
                                            linear_algebra::JacobianMultiplier,
                                            const linear_algebra::DynamicVector<double>&>;

    ///@brief Construct a new ROLVectorConstraintFunction object
    ROLVectorConstraintFunction(
        criteria::library::VectorConstraint<const linear_algebra::DynamicVector<double>&> aConstraint);

    ///@brief Evaluate and populate aConstraints with the constraints at a given control vector and tolerance
    void value(std::vector<double>& aConstraints, const std::vector<double>& aControl, double& aTolerance) override;

    ///@brief Evaluate the Jacobian in a given direction for the set of controls. Populate aJacobianTimesDirection
    void applyJacobian(std::vector<double>& aJacobianTimesDirection,
                       const std::vector<double>& aDirection,
                       const std::vector<double>& aControl,
                       double& aTolerance) override;

    ///@brief Evaluate the adjoint Jacobian times a direction for the set of controls.
    ///@param aAdjointJacobianTimesDirection Stores result
    void applyAdjointJacobian(std::vector<double>& aAdjointJacobianTimesDirection,
                              const std::vector<double>& aDual,
                              const std::vector<double>& aControl,
                              double& aTolerance) override;

    /*  void applyAdjointHessian(std::vector<double>& ahuv,
                               const std::vector<double>& u,
                               const std::vector<double>& v,
                               const std::vector<double>& x,
                               double& tol) override;*/

   private:
    ROLPlatoFunction mFunctionWithDfAsJacobian;
    ROLPlatoFunction mFunctionWithDfAsAdjointJacobian;
    double mConstraintTarget = 0;
};
}  // namespace plato::third_party_integration::rol

#endif
