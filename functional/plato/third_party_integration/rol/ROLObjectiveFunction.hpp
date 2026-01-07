#ifndef PLATO_ROL_INTEGRATION_ROLOBJECTIVE
#define PLATO_ROL_INTEGRATION_ROLOBJECTIVE

#include <ROL_StdObjective.hpp>
#include <ROL_UpdateType.hpp>
#include <ROL_Vector.hpp>

#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/output/OutputManager.hpp"

namespace plato::third_party_integration::rol
{
/// @brief Objective interface with ROL.
class ROLObjectiveFunction : public ROL::StdObjective<double>
{
   public:
    using ROLPlatoFunction =
        core::Function<const linear_algebra::DynamicVector<double>&,
                       core::FunctionInfo<double, core::evaluation::kFunction>,
                       core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFirstDerivative>>;

    ///@brief Construct a new ROLObjectiveFunction object
    ROLObjectiveFunction(ROLPlatoFunction aROLPlatoFunction, output::OutputManager aOutputManager = {});

    ///@brief Write geometry output
    void update(const std::vector<double>& aControls, ROL::UpdateType aIterationType, int aIteration) override;

    ///@brief Compute the value using the set of controls @a aControls and given tolerance @a aTolerance
    double value(const std::vector<double>& aControls, double& aTolerance) override;

    ///@brief Compute the gradient from the set of controls and given tolerance. Populate the aGradient parameter
    ///@param aGradient Modify this variable with the gradient
    void gradient(std::vector<double>& aGradient, const std::vector<double>& aControls, double& aTolerance) override;

    ///@brief Compute the hessian times a vector @a aVector from the set of controls @a aControls, and given tolerance
    ///@a aTolerance. Populate @a aHessianTimesVector parameter
    /// This function is hard coded to return a zero Hessian.
    void hessVec(std::vector<double>& aHessianTimesVector,
                 const std::vector<double>& aVector,
                 const std::vector<double>& aControls,
                 double& aTolerance) override;

    ///@brief Write geometry output after optimization has concluded
    void finalUpdate(const linear_algebra::DynamicVector<double>& aControls);

   private:
    ROLPlatoFunction mFunction;
    output::OutputManager mOutputManager;
};
}  // namespace plato::third_party_integration::rol

#endif
