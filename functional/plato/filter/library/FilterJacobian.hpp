#ifndef PLATO_FILTER_LIBRARY_FILTERJACOBIAN
#define PLATO_FILTER_LIBRARY_FILTERJACOBIAN

#include <memory>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::filter::library
{
class FilterInterface;

using FilterJacobian = linear_algebra::JacobianMultiplier;
using FilterAdjointJacobian = linear_algebra::AdjointJacobianMultiplier;

/// @brief Creates a JacobianMultiplier from a FilterInterface object, evaluated at @a aAnalysisDomainMesh.
/// @pre @a aFilter must not be `nullptr`.
auto make_filter_jacobian(std::shared_ptr<FilterInterface> aFilter, analysis::AnalysisDomainMesh aAnalysisDomainMesh)
    -> linear_algebra::JacobianMultiplier;

/// @brief Creates an AdjointJacobianMultiplier from a FilterInterface object, evaluated at @a aAnalysisDomainMesh.
/// @pre @a aFilter must not be `nullptr`.
auto make_filter_adjoint_jacobian(std::shared_ptr<FilterInterface> aFilter,
                                  analysis::AnalysisDomainMesh aAnalysisDomainMesh)
    -> linear_algebra::AdjointJacobianMultiplier;

}  // namespace plato::filter::library

#endif
