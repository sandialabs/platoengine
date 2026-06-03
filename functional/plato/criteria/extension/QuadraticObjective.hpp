#ifndef PLATO_CRITERIA_EXTENSION_QUADRATICOBJECTIVE
#define PLATO_CRITERIA_EXTENSION_QUADRATICOBJECTIVE

#include <string_view>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::criteria::extension
{
/// @brief Computes the quadratic sum of all nodal or element centered densities.
struct QuadraticObjective
{
    [[nodiscard]] static auto value(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> double;
    [[nodiscard]] static auto gradient(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        -> linear_algebra::DynamicVector<double>;

    static constexpr auto mCriterionName = std::string_view{"quadratic"};
};

/// @brief Creates a Function object from a QuadraticObjective
[[nodiscard]] auto make_quadratic_objective_function() -> library::CriterionFunction;

}  // namespace plato::criteria::extension

#endif
