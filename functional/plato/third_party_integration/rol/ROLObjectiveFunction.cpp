#include "plato/third_party_integration/rol/ROLObjectiveFunction.hpp"

namespace plato::third_party_integration::rol
{
ROLObjectiveFunction::ROLObjectiveFunction(ROLPlatoFunction aROLPlatoFunction) : mFunction(std::move(aROLPlatoFunction))
{
}

double ROLObjectiveFunction::value(const std::vector<double>& aControls, double&)
{
    return mFunction.evaluate<core::evaluation::kFunction>(linear_algebra::DynamicVector<double>{aControls});
}

void ROLObjectiveFunction::gradient(std::vector<double>& aGradient, const std::vector<double>& aControls, double&)
{
    aGradient = mFunction.evaluate<core::evaluation::kFirstDerivative>(linear_algebra::DynamicVector<double>{aControls})
                    .stdVector();
}

void ROLObjectiveFunction::hessVec(std::vector<double>& aHessianTimesVector,
                                   const std::vector<double>& /*aVector*/,
                                   const std::vector<double>& /*aControls*/,
                                   double& /*aTolerance*/)
{
    aHessianTimesVector = std::vector<double>(aHessianTimesVector.size(), 0.0);
}

}  // namespace plato::third_party_integration::rol
