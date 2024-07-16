#include "plato/third_party_integration/rol/ROLObjectiveFunction.hpp"

#include <ROL_StdVector.hpp>

#include "plato/third_party_integration/rol/Utilities.hpp"

namespace plato::third_party_integration::rol
{
ROLObjectiveFunction::ROLObjectiveFunction(ROLPlatoFunction aROLPlatoFunction) : mFunction(std::move(aROLPlatoFunction))
{
}

double ROLObjectiveFunction::value(const ROL::Vector<double>& aControls, double&)
{
    return mFunction.f(to_dynamic_vector(aControls));
}

void ROLObjectiveFunction::gradient(ROL::Vector<double>& aGradient, const ROL::Vector<double>& aControls, double&)
{
    linear_algebra::DynamicVector<double> tLocalGradient = mFunction.df(to_dynamic_vector(aControls));
    assign_vector(aGradient, std::move(tLocalGradient).stdVector());
}

}  // namespace plato::third_party_integration::rol
