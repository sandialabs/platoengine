#ifndef PLATO_CRITERIA_EXTENSION_NODALSUMOBJECTIVE
#define PLATO_CRITERIA_EXTENSION_NODALSUMOBJECTIVE

#include <string_view>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::criteria::extension
{
/// @brief Computes the sum of all nodal coordinates. The main purpose of this
///  class is for implementing a geometry sensitivity check.
struct NodalSumObjective
{
    [[nodiscard]] auto f(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const -> double;
    [[nodiscard]] auto df(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
        -> linear_algebra::DynamicVector<double>;

    static constexpr auto kCriterionName = std::string_view{"nodal_sum"};
};

/// @brief Creates a Function object from a NodalSumObjective
[[nodiscard]] auto make_nodal_sum_function() -> library::CriterionFunction;

}  // namespace plato::criteria::extension

#endif
