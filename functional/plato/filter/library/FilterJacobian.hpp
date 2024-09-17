#ifndef PLATO_FILTER_LIBRARY_FILTERJACOBIAN
#define PLATO_FILTER_LIBRARY_FILTERJACOBIAN

#include <memory>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::filter::library
{
class FilterInterface;

/// @brief A wrapper for a filter an a filter argument, used for implementing operator*.
struct FilterJacobian
{
    std::shared_ptr<FilterInterface> mFilter;
    analysis::AnalysisDomainMesh mAnalysisDomainMesh;
};

/// @pre `FilterJacobian::mFilter` must not be `nullptr`.
linear_algebra::DynamicVector<double> operator*(const linear_algebra::DynamicVector<double>& aV,
                                                const FilterJacobian& aJacobian);

}  // namespace plato::filter::library
#endif
