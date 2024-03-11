#ifndef PLATO_ROL_INTEGRATION_ROLCONSTRAINT
#define PLATO_ROL_INTEGRATION_ROLCONSTRAINT

#include <ROL_Constraint.hpp>
#include <ROL_Vector.hpp>

#include "plato/core/Function.hpp"
#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::rol_integration
{
class ROLConstraintFunction : public ROL::Constraint<double>
{
   public:
    using ROLPlatoFunction =
        core::Function<double, linear_algebra::DynamicVector<double>, const linear_algebra::DynamicVector<double>&>;

    ///@brief Construct a new ROLConstraintFunction object
    ROLConstraintFunction(criteria::library::Constraint<const linear_algebra::DynamicVector<double>&> aConstraint);

    ///@brief Evaluate and populate aConstraints with the constraints at a given control vector and tolerance
    void value(ROL::Vector<double>& aConstraints, const ROL::Vector<double>& aControl, double& aTolerance) override;

    ///@brief Evaluate the Jacobian in a given direction for the set of controls. Populate aJacobianTimesDirection
    void applyJacobian(ROL::Vector<double>& aJacobianTimesDirection,
                       const ROL::Vector<double>& aDirection,
                       const ROL::Vector<double>& aControl,
                       double& aTolerance) override;

    ///@brief Evaluate the adjoint Jacobian times a direction for the set of controls.
    ///@param aAdjointJacobianTimesDirection Stores result
    void applyAdjointJacobian(ROL::Vector<double>& aAdjointJacobianTimesDirection,
                              const ROL::Vector<double>& aDual,
                              const ROL::Vector<double>& aControl,
                              double& aTolerance) override;

    void applyAdjointJacobian(ROL::Vector<double>& aAdjointJacobianTimesDirection,
                              const ROL::Vector<double>& aDual,
                              const ROL::Vector<double>& aControl,
                              const ROL::Vector<double>& aDualV,
                              double& aTolerance) override;

    ///@brief Is this constraint linear
    bool linear() const;

    ///@brief return the name of the constraint for output
    const std::string& name() const;

   private:
    std::string mName;
    ROLPlatoFunction mFunction;
    double mConstraintTarget = 0;
    bool mLinear = false;
};
}  // namespace plato::rol_integration

#endif
