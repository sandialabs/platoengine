#ifndef PLATO_CRITERIA_EXTENSION_VOLUMECRITERION
#define PLATO_CRITERIA_EXTENSION_VOLUMECRITERION

#include <string_view>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::criteria::extension
{
/// @brief Computes the volume of all elements and scales that value by a scale factor [ie density]. The main purpose of
/// this class is for implementing a volume or mass constraint. This class will assume the densities are element
/// centered controls and not nodal
struct VolumeCriterion
{
    [[nodiscard]] double f(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const;
    [[nodiscard]] linear_algebra::DynamicVector<double> df(
        const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const;

    static constexpr auto kVolumeCriterionName = std::string_view{"volume"};
    static constexpr auto kVolumeFractionCriterionName = std::string_view{"volume_fraction"};

    double mScaleFactor = 1;
};

/// @brief Creates a Function object from a VolumeCriterion
[[nodiscard]] auto make_volume_constraint_function()
    -> core::Function<double, linear_algebra::DynamicVector<double>, const analysis::AnalysisDomainMesh&>;

/// @brief Creates a Function object from a VolumeCriterion
[[nodiscard]] auto make_volume_fraction_constraint_function()
    -> core::Function<double, linear_algebra::DynamicVector<double>, const analysis::AnalysisDomainMesh&>;

}  // namespace plato::criteria::extension

#endif
