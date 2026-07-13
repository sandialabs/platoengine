#include "plato/filter/extension/kernel_filters/WedgeFilter.hpp"

#include <boost/mpi/communicator.hpp>
#include <cmath>
#include <filesystem>
#include <numbers>
#include <optional>
#include <string>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/filter/extension/CommonInputValidation.hpp"
#include "plato/filter/extension/FilterMeshUtilities.hpp"
#include "plato/filter/extension/kernel_filters/CylindricalMaskUtilities.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilter.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilterCenteringTypes.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilterInputUtilities.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilder.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilderUtilities.hpp"
#include "plato/filter/extension/kernel_filters/ReflectionUtilities.hpp"
#include "plato/filter/extension/kernel_filters/ReliableMeshZeroValue.hpp"
#include "plato/filter/extension/kernel_filters/SphericalMaskUtilities.hpp"
#include "plato/filter/extension/kernel_filters/Wedge.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/FixedBlockUtilities.hpp"
#include "plato/third_party_integration/common/BoundingBox.hpp"
#include "plato/third_party_integration/stk_search/Utilities.hpp"
#include "plato/utilities/BoundClusivity.hpp"
#include "plato/utilities/ParameterBounds.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::filter::extension::kernel_filters
{

namespace
{
constexpr auto kDegreesPerCircle = 360.0;
constexpr auto kDegreesPerRadian = 180.0 / std::numbers::pi;

const auto kMakeWedgeFilterInfo = [](const input_parser::wedge_filter& aInput,
                                     const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> WedgeFilterType
{ return make_wedge_filter_type(aInput, aAnalysisDomainMesh); };

[[maybe_unused]] const auto kWedgeFilterParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::wedge_filter>{};

[[maybe_unused]] const auto kWedgeFilterRegistration = library::FilterRegistration{
    input_parser::block_name<input_parser::wedge_filter>(), [](const library::ValidatedFilterInput& aInput)
    {
        const auto& tInput = input_validation::get_input_block<input_parser::wedge_filter>(aInput);
        return library::make_filter_function_from_cache(
            [&tInput]()
            { return detail::create_kernel_filter_cache<input_parser::wedge_filter>(tInput, kMakeWedgeFilterInfo); });
    }};

[[maybe_unused]] const auto kWedgeFilterValidationRegistration =
    validation_for_all_kernel_filters<input_parser::wedge_filter>();

[[maybe_unused]] const auto kWedgeFilterTargetMeshValidationRegistration =
    target_mesh_validation_for_all_kernel_filters<input_parser::wedge_filter>();

[[maybe_unused]] const auto kWedgeFilterValidationRegistrationSpecialized =
    input_validation::InputBlockValidationRegistration<>{
        [](const input_parser::wedge_filter& aInput)
        { return detail::validate_wedge_angle_commensurate_with_360(aInput); },
        [](const input_parser::wedge_filter& aInput) { return detail::validate_wedge_angle_even_dihedral(aInput); },
        [](const input_parser::wedge_filter& aInput)
        {
            return input_validation::error_message_for_parameter_out_of_bounds(
                "wedge_filter", aInput.wedge_angle, "wedge_angle", utilities::lower_bounded(utilities::Exclusive{0.0}));
        }};

[[maybe_unused]] const auto kWedgeFilterMeshBasedValidationRegistration =
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

    return WedgeFilterType{
        .mKernelFilterFunctions =
            KernelFilterFunctions{.mRadius = aRadius,
                                  .mFilterCentering = aFilterCentering,
                                  .mSearchFunction = tSearchFunction,
                                  .mUnnormalizedWeightFunction = tUnnormalizedWeightFunction,
                                  .mLoggingName = input_parser::block_name<input_parser::wedge_filter>()},
        .mTargetMeshFunction = tTargetMeshFunction};
}

auto validate_wedge_angle_commensurate_with_360(const input_parser::wedge_filter& aInput) -> std::optional<std::string>
{
    if (!aInput.wedge_angle)
    {
        return input_validation::error_message_for_empty_parameter(
            input_parser::block_name<input_parser::wedge_filter>(), aInput.wedge_angle, "wedge_angle");
    }
    if (std::fmod(kDegreesPerCircle, aInput.wedge_angle.value()) != 0)
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
    if (static_cast<int>(kDegreesPerCircle / aInput.wedge_angle.value()) % 2 != 0)
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
    const auto tMaximumAngleInDegrees = tBoundingBox.second.y * kDegreesPerRadian;
    if (std::fabs(tMaximumAngleInDegrees - aInput.wedge_angle.value()) >= reliable_mesh_zero_value())
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
