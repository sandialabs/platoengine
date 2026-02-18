#include "plato/filter/extension/kernel_filters/WedgeFilter.hpp"

#include <boost/mpi.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/vector.hpp>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/filter/extension/CommonInputValidation.hpp"
#include "plato/filter/extension/FilterMeshUtilities.hpp"
#include "plato/filter/extension/kernel_filters/CylindricalMaskUtilities.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilter.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilterInputUtilities.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilder.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilderUtilities.hpp"
#include "plato/filter/extension/kernel_filters/ReflectionUtilities.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/FixedBlockUtilities.hpp"

namespace plato::filter::extension::kernel_filters
{

namespace
{
const auto kMakeWedgeFilterInfo = [](const input_parser::wedge_filter& aInput,
                                     const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> WedgeFilterType
{ return make_wedge_filter_type(aInput, aAnalysisDomainMesh); };

[[maybe_unused]] static auto kWedgeFilterParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::wedge_filter>{};

[[maybe_unused]] static auto kWedgeFilterRegistration = library::FilterRegistration{
    input_parser::block_name<input_parser::wedge_filter>(), [](const library::ValidatedFilterInput& aInput)
    {
        const auto& tInput = input_validation::get_input_block<input_parser::wedge_filter>(aInput);
        return library::make_filter_function_from_cache(
            [&tInput]()
            { return detail::create_kernel_filter_cache<input_parser::wedge_filter>(tInput, kMakeWedgeFilterInfo); });
    }};

[[maybe_unused]] static auto kWedgeFilterValidationRegistration =
    validation_for_all_kernel_filters<input_parser::wedge_filter>();

[[maybe_unused]] static auto kWedgeFilterTargetMeshValidationRegistration =
    target_mesh_validation_for_all_kernel_filters<input_parser::wedge_filter>();

[[maybe_unused]] static auto kWedgeFilterValidationRegistrationSpecialized =
    input_validation::InputBlockValidationRegistration<>{
        [](const input_parser::wedge_filter& aInput)
        { return detail::validate_wedge_angle_commensurate_with_360(aInput); },
        [](const input_parser::wedge_filter& aInput) { return detail::validate_wedge_angle_even_dihedral(aInput); },
        [](const input_parser::wedge_filter& aInput)
        {
            return input_validation::error_message_for_parameter_out_of_bounds(
                "wedge_filter", aInput.wedge_angle, "wedge_angle", utilities::lower_bounded(utilities::Exclusive{0.0}));
        }};

[[maybe_unused]] static auto kWedgeFilterMeshBasedValidationRegistration =
    input_validation::InputBlockValidationRegistration<std::filesystem::path>{
        [](const input_parser::wedge_filter& aInput, const std::filesystem::path& aMeshPath)
        { return extension::detail::validate_filter_radius_with_mesh(aInput, aMeshPath); },
        [](const input_parser::wedge_filter& aInput, const std::filesystem::path& aMeshPath)
        { return detail::validate_source_mesh_matches_wedge_angle(aInput, aMeshPath); },
        [](const input_parser::wedge_filter& aInput, const std::filesystem::path& aMeshPath)
        { return detail::validate_source_mesh_wedge_positive_y(aInput, aMeshPath); },
        [](const input_parser::wedge_filter& aInput, const std::filesystem::path& aMeshPath)
        {
            return validate_all_target_domain_find_source_domain<input_parser::wedge_filter>(
                aInput, aMeshPath, kMakeWedgeFilterInfo, boost::mpi::communicator{});
        }};

}  // namespace

auto make_wedge_filter_type(const input_parser::wedge_filter& aInput,
                            const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> WedgeFilterType
{
    const auto tFilterRadius = extension::detail::get_filter_radius(aInput, aAnalysisDomainMesh.mFileName);
    return detail::make_wedge_filter_type(tFilterRadius, aInput.centering_type.value(), aInput.wedge_angle.value());
}

namespace detail
{
auto make_wedge_filter_type(const double aRadius,
                            const input_parser::KernelFilterCenteringTypes aFilterCentering,
                            const double aWedgeAngle) -> WedgeFilterType
{
    const auto tWedge = positive_quadrant_wedge(aWedgeAngle);
    const auto tSearchFunction =
        [aRadius, tWedge](
            const TargetRowVector& aTargetRowVector, const SourceColumnVector& aSourceColumnVector,
            const boost::mpi::communicator& aLambdaCommunicator) -> third_party_integration::stk_search::SearchResults
    {
        return distribute_search_vectors_and_stk_search_with_spheres(aTargetRowVector, aSourceColumnVector, aRadius,
                                                                     aLambdaCommunicator, tWedge);
    };

    const auto tUnnormalizedWeightFunction = [aRadius, tWedge](const SourcePoint& aSourcePoint,
                                                               const TargetPoint& aTargetPoint) -> double
    {
        const double tDistance = minimum_distance_reflected_points_wedge(aSourcePoint, aTargetPoint, tWedge);
        return detail::linear_ramp_weight(Distance{tDistance}, SearchRadius{aRadius});
    };

    const auto tTargetMeshFunction = [](const input_parser::wedge_filter& aInput) -> mesh::Mesh
    {
        const auto tFixedBlocks = mesh::fixed_blocks(aInput);
        return mesh::Mesh{aInput.target_mesh_name.value().mToken, tFixedBlocks};
    };

    return WedgeFilterType{aRadius, aFilterCentering, tSearchFunction, tUnnormalizedWeightFunction,
                           tTargetMeshFunction};
}

auto validate_wedge_angle_commensurate_with_360(const input_parser::wedge_filter& aInput) -> std::optional<std::string>
{
    if (!aInput.wedge_angle)
    {
        return input_validation::error_message_for_empty_parameter(
            input_parser::block_name<input_parser::wedge_filter>(), aInput.wedge_angle, "wedge_angle");
    }
    if (std::fmod(360, aInput.wedge_angle.value()) != 0)
    {
        return std::optional<std::string>{utilities::concatenate(input_parser::block_name<input_parser::wedge_filter>(),
                                                                 " wedge angle must evenly divide 360 degrees.")};
    }
    return std::nullopt;
}

auto validate_wedge_angle_even_dihedral(const input_parser::wedge_filter& aInput) -> std::optional<std::string>
{
    if (!aInput.wedge_angle)
    {
        return std::nullopt;  // Not our error
    }
    if (static_cast<int>(360 / aInput.wedge_angle.value()) % 2 != 0)
    {
        return std::optional<std::string>{
            utilities::concatenate(input_parser::block_name<input_parser::wedge_filter>(),
                                   " wedge angle must divide 360 degrees in an even number of wedges.")};
    }
    return std::nullopt;
}

auto validate_source_mesh_matches_wedge_angle(const input_parser::wedge_filter& aInput,
                                              const std::filesystem::path& aSourceMeshFileName)
    -> std::optional<std::string>
{
    if (!std::filesystem::exists(aSourceMeshFileName) || !aInput.wedge_angle.has_value())
    {
        return std::nullopt;  // Not our error
    }
    const auto tPolarCoordinates =
        cylindrical_polar_coordinates(mesh::EntityRetrieval{mesh::Mesh{aSourceMeshFileName}}.nodalCoordinates()).mValue;
    const auto tBoundingBox = third_party_integration::common::bounding_box(tPolarCoordinates);
    const auto tMaximumAngleInDegrees = tBoundingBox.second.y * 180.0 / std::numbers::pi;
    if (constexpr auto tAngleTolerance = 1e-6;
        std::fabs(tMaximumAngleInDegrees - aInput.wedge_angle.value()) >= tAngleTolerance)
    {
        return std::optional<std::string>{utilities::concatenate(
            input_parser::block_name<input_parser::wedge_filter>(),
            ": source mesh wedge angle does not equal input specified ", std::to_string(aInput.wedge_angle.value()),
            " degrees.  Found an angle of: ", std::to_string(tMaximumAngleInDegrees), " degrees.")};
    }

    return std::nullopt;
}

auto validate_source_mesh_wedge_positive_y(const input_parser::wedge_filter& aInput,
                                           const std::filesystem::path& aSourceMeshFileName)
    -> std::optional<std::string>
{
    if (!std::filesystem::exists(aSourceMeshFileName) || !aInput.wedge_angle.has_value())
    {
        return std::nullopt;  // Not our error
    }
    const auto tCoordinates = mesh::EntityRetrieval{mesh::Mesh{aSourceMeshFileName}}.nodalCoordinates();
    const auto tBoundingBox = third_party_integration::common::bounding_box(tCoordinates);
    const auto tMinimumYValue = tBoundingBox.first.y;

    if (constexpr auto tMeshReliabilityThreshold = 1e-6; tMinimumYValue < -tMeshReliabilityThreshold)
    {
        return std::optional<std::string>{utilities::concatenate(
            input_parser::block_name<input_parser::wedge_filter>(),
            ": source mesh wedge angle does not start on x-axis, found y values less than zero: ",
            std::to_string(tMinimumYValue), " degrees.")};
    }
    return std::nullopt;
}

}  // namespace detail

}  // namespace plato::filter::extension::kernel_filters
