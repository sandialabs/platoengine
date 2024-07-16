#include "plato/third_party_integration/rol/Utilities.hpp"

namespace plato::third_party_integration::rol
{

ROL::StdVector<double> generate_perturbation(const int aDimension)
{
    ROL::StdVector<double> tPerturbation(aDimension);
    tPerturbation.randomize(-1, 1);
    tPerturbation.scale(1.0 / tPerturbation.norm());
    return tPerturbation;
}

}  // namespace plato::third_party_integration::rol
