#ifndef PLATO_CRITERIA_EXTENSION_NODALSUMOBJECTIVE
#define PLATO_CRITERIA_EXTENSION_NODALSUMOBJECTIVE

#include <string_view>

#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/mesh/MeshDesignVariables.hpp"

namespace plato::criteria::extension
{
/// @brief Computes the sum of all nodal coordinates. The main purpose of this
///  class is for implementing a geometry sensitivity check.
struct NodalSumObjective
{
    [[nodiscard]] double f(const mesh::MeshDesignVariables& aMeshDesignVariables) const;
    [[nodiscard]] linear_algebra::DynamicVector<double> df(const mesh::MeshDesignVariables& aMeshDesignVariables) const;

    static constexpr auto kCriterionName = std::string_view{"nodal_sum"};
};

/// @brief Creates a Function object from a NodalSumObjective
[[nodiscard]] auto make_nodal_sum_function()
    -> core::Function<double, linear_algebra::DynamicVector<double>, const mesh::MeshDesignVariables&>;

}  // namespace plato::criteria::extension

#endif
