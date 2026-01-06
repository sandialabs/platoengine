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
/// @brief Implementation of a ROL::StdConstraint that represents a vector-valued constraint.
///
/// This is the main interface between plato and ROL constraints.
class ROLVectorConstraintFunction : public ROL::StdConstraint<double>
{
   public:
    using ROLPlatoFunction =
        core::Function<const linear_algebra::DynamicVector<double>&,
                       core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFunction>,
                       core::FunctionInfo<linear_algebra::JacobianMultiplier, core::evaluation::kFirstDerivative>,
                       core::FunctionInfo<linear_algebra::AdjointJacobianMultiplier,
                                          core::evaluation::kFirstDerivative,
                                          core::MatrixOrdering::kAdjoint>>;

    ///@brief Construction from a Function that computes a constraint and its Jacobian.
    ROLVectorConstraintFunction(ROLPlatoFunction aConstraint);

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

    ///@brief Evaluate the Adjoint Hessian at @a aControl applied to vector @a v
    ///@param aHessianUV Stores result
    /// This function is hardcoded to return 0s.
    void applyAdjointHessian(std::vector<double>& aHessianUV,
                             const std::vector<double>& u,
                             const std::vector<double>& v,
                             const std::vector<double>& aControl,
                             double& aTolerance) override;

   private:
    ROLPlatoFunction mFunction;
};
}  // namespace plato::third_party_integration::rol

#endif
