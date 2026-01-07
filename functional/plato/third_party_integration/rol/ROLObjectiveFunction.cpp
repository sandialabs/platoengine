#include "plato/third_party_integration/rol/ROLObjectiveFunction.hpp"

namespace plato::third_party_integration::rol
{
ROLObjectiveFunction::ROLObjectiveFunction(ROLPlatoFunction aROLPlatoFunction, output::OutputManager aOutputManager)
    : mFunction(std::move(aROLPlatoFunction)), mOutputManager(std::move(aOutputManager))
{
}

void ROLObjectiveFunction::update(const std::vector<double>& aControls,
                                  ROL::UpdateType aIterationType,
                                  int /*aIteration*/)
{
    if (aIterationType == ROL::UpdateType::Initial || aIterationType == ROL::UpdateType::Accept)
    {
        mOutputManager.output(linear_algebra::DynamicVector<double>{aControls});
    }
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

void ROLObjectiveFunction::finalUpdate(const linear_algebra::DynamicVector<double>& aControls)
{
    mOutputManager.output(aControls);
}

}  // namespace plato::third_party_integration::rol
