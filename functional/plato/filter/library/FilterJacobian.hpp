#ifndef PLATO_FILTER_LIBRARY_FILTERJACOBIAN
#define PLATO_FILTER_LIBRARY_FILTERJACOBIAN

#include <memory>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::filter::library
{
class FilterInterface;

/// @brief A wrapper for a filter and a filter argument, used for implementing operator*.
struct FilterJacobian
{
    std::shared_ptr<FilterInterface> mFilter;
    analysis::AnalysisDomainMesh mAnalysisDomainMesh;
};

using FilterAdjointJacobian = utilities::NamedType<FilterJacobian, struct FilterAdjointJacobianTag>;

/// @pre `FilterJacobian::mFilter` must not be `nullptr`.
[[nodiscard]] auto operator*(const linear_algebra::DynamicVector<double>& aV, const FilterJacobian& aJacobian)
    -> linear_algebra::DynamicVector<double>;

/// @pre `FilterJacobian::mFilter` must not be `nullptr`.
[[nodiscard]] auto operator*(const linear_algebra::DynamicVector<double>& aV, const FilterAdjointJacobian& aJacobian)
    -> linear_algebra::DynamicVector<double>;

}  // namespace plato::filter::library
#endif
