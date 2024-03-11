#ifndef PLATO_CRITERIA_EXTENSION_NODALSUMOBJECTIVE
#define PLATO_CRITERIA_EXTENSION_NODALSUMOBJECTIVE

#include "plato/core/Function.hpp"
#include "plato/core/MeshProxy.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::criteria::extension
{
/// @brief Computes the sum of all nodal coordinates. The main purpose of this
///  class is for implementing a geometry sensitivity check.
struct NodalSumObjective
{
    [[nodiscard]] double f(const core::MeshProxy& aMeshProxy) const;
    [[nodiscard]] linear_algebra::DynamicVector<double> df(const core::MeshProxy& aMeshProxy) const;
};

/// @brief Creates a Function object from a NodalSumObjective
[[nodiscard]] auto make_nodal_sum_function()
    -> core::Function<double, linear_algebra::DynamicVector<double>, const core::MeshProxy&>;

}  // namespace plato::criteria::extension

#endif
