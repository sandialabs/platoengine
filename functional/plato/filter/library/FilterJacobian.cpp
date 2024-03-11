#include "plato/filter/library/FilterJacobian.hpp"

#include <cassert>

#include "plato/filter/library/FilterInterface.hpp"

namespace plato::filter::library
{
linear_algebra::DynamicVector<double> operator*(const linear_algebra::DynamicVector<double>& aV,
                                                const FilterJacobian& aJacobian)
{
    assert(aJacobian.mFilter);
    return aJacobian.mFilter->jacobianTimesVector(aJacobian.mMeshProxy, aV);
}
}  // namespace plato::filter::library
