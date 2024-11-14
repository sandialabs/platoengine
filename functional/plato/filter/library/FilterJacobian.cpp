#include "plato/filter/library/FilterJacobian.hpp"

#include <cassert>

#include "plato/filter/library/FilterInterface.hpp"

namespace plato::filter::library
{
auto operator*(const linear_algebra::DynamicVector<double>& aV, const FilterJacobian& aJacobian)
    -> linear_algebra::DynamicVector<double>
{
    assert(aJacobian.mFilter);
    return aJacobian.mFilter->rowVectorTimesJacobian(aJacobian.mAnalysisDomainMesh, aV);
}

auto operator*(const linear_algebra::DynamicVector<double>& aV, const FilterAdjointJacobian& aAdjointJacobian)
    -> linear_algebra::DynamicVector<double>
{
    assert(aAdjointJacobian.mValue.mFilter);
    return aAdjointJacobian.mValue.mFilter->rowVectorTimesAdjointJacobian(aAdjointJacobian.mValue.mAnalysisDomainMesh,
                                                                          aV);
}

}  // namespace plato::filter::library
