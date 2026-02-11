#ifndef PLATO_CRITERIA_EXTENSION_VOLUMECRITERION
#define PLATO_CRITERIA_EXTENSION_VOLUMECRITERION

#include <boost/fusion/include/define_struct.hpp>
#include <string_view>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

///@brief Input struct for specifying blocks over which volume should be computed
// clang-format off
BOOST_FUSION_DEFINE_STRUCT(
(plato)(input_parser),volume_criterion,
(boost::optional<bool>, ignore_void_blocks)
(boost::optional<double>, reference_volume)
)
// clang-format on

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
    bool mIgnoreVoidBlocks;
};

/// @brief Creates a Function object from a VolumeCriterion
[[nodiscard]] auto make_volume_constraint_function(const bool aIgnoreVoidBlocks) -> library::CriterionFunction;

/// @brief Creates a Function object from a VolumeCriterion
[[nodiscard]] auto make_volume_fraction_constraint_function(const bool aIgnoreVoidBlocks,
                                                            const boost::optional<double> aReferenceVolume)
    -> library::CriterionFunction;

namespace detail
{
auto parse_input_block(const std::filesystem::path& aFilename) -> input_parser::volume_criterion;

[[nodiscard]] double reference_volume(const boost::optional<double> aReferenceVolume,
                                      const analysis::AnalysisDomainMesh& aAnalysisDomainMesh);
}  // namespace detail

}  // namespace plato::criteria::extension

#endif
