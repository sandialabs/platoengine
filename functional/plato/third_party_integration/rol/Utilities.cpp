#include "plato/third_party_integration/rol/Utilities.hpp"

namespace plato::third_party_integration::rol
{

ROL::StdVector<double> generate_perturbation(const int aDimension)
{
    ROL::StdVector<double> tPerturbation(aDimension);
    randomize_and_normalize(tPerturbation);
    return tPerturbation;
}

void randomize_and_normalize(ROL::Vector<double>& aROLVector)
{
    aROLVector.randomize(-1.0, 1.0);
    aROLVector.scale(1.0 / aROLVector.norm());
}

}  // namespace plato::third_party_integration::rol
