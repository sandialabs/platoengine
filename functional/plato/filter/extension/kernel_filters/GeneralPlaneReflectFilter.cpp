#include "plato/filter/extension/kernel_filters/GeneralPlaneReflectFilter.hpp"

#include <boost/mpi/communicator.hpp>
#include <cstddef>
#include <filesystem>
#include <iterator>
#include <limits>
#include <optional>
#include <string>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/filter/extension/CommonInputValidation.hpp"
#include "plato/filter/extension/FilterMeshUtilities.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilter.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilterCenteringTypes.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilterInputUtilities.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilder.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilderUtilities.hpp"
#include "plato/filter/extension/kernel_filters/Plane.hpp"
#include "plato/filter/extension/kernel_filters/ReflectionUtilities.hpp"
#include "plato/filter/extension/kernel_filters/ReliableMeshZeroValue.hpp"
#include "plato/filter/extension/kernel_filters/SphericalMaskUtilities.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/mesh/FixedBlockUtilities.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/stk_search/Utilities.hpp"
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/StringUtilities.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::filter::extension::kernel_filters
{

namespace
{
constexpr auto kValidationOutputPrecision = 8U;

const auto kMakeReflectFilterInfo =
    [](const input_parser::general_plane_reflect_filter& aInput,
       const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> GeneralPlaneReflectFilterType
{ return make_general_plane_reflect_filter_type(aInput, aAnalysisDomainMesh); };

[[maybe_unused]] const auto kGeneralReflectFilterParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::general_plane_reflect_filter>{};

[[maybe_unused]] const auto kGeneralReflectFilterRegistration = library::FilterRegistration{
    input_parser::block_name<input_parser::general_plane_reflect_filter>(),
    [](const library::ValidatedFilterInput& aInput)
    {
        const auto& tInput = input_validation::get_input_block<input_parser::general_plane_reflect_filter>(aInput);
        return library::make_filter_function_from_cache(
            [&tInput]()
            {
                return detail::create_kernel_filter_cache<input_parser::general_plane_reflect_filter>(
                    tInput, kMakeReflectFilterInfo);
            });
    }};

[[maybe_unused]] const auto kGeneralReflectFilterDefaultValidationRegistration =
    validation_for_all_kernel_filters<input_parser::general_plane_reflect_filter>();

[[maybe_unused]] const auto kGeneralReflectFilterValidationRegistration =
    input_validation::InputBlockValidationRegistration<>{
        [](const input_parser::general_plane_reflect_filter& aInput)
        { return detail::validate_required_plane_normal_is_non_zero_magnitude(aInput); },
        [](const input_parser::general_plane_reflect_filter& aInput)
        { return detail::validate_required_plane(aInput); }};

[[maybe_unused]] const auto kGeneralReflectFilterTargetMeshValidationRegistration =
    target_mesh_validation_for_all_kernel_filters<input_parser::general_plane_reflect_filter>();

[[maybe_unused]] const auto kGeneralReflectFilterMeshBasedValidationRegistration =
    input_validation::InputBlockValidationRegistration<std::filesystem::path>{
        [](const input_parser::general_plane_reflect_filter& aInput, const std::filesystem::path& aMeshPath)
        { return extension::detail::validate_filter_radius_with_mesh(aInput, aMeshPath); },
        [](const input_parser::general_plane_reflect_filter& aInput, const std::filesystem::path& aMeshPath)
        { return detail::validate_source_mesh_on_outward_normal_side_of_plane(aInput, aMeshPath); },
        [](const input_parser::general_plane_reflect_filter& aInput, const std::filesystem::path& aMeshPath)
        {
            return validate_all_target_domain_find_source_domain<input_parser::general_plane_reflect_filter>(
                aInput, aMeshPath, kMakeReflectFilterInfo, boost::mpi::communicator{});
        }};

[[nodiscard]] auto center_coordinate(const input_parser::general_plane_reflect_filter& aInput)
    -> third_party_integration::common::Coordinate
{
    const auto tPoint = aInput.plane.value().center;
    return {tPoint.mX, tPoint.mY, tPoint.mZ};
}

[[nodiscard]] auto plane_normal(const input_parser::general_plane_reflect_filter& aInput)
    -> third_party_integration::common::UnitVector3
{
    const auto tNormal = aInput.plane.value().normal;
    return {tNormal.mX, tNormal.mY, tNormal.mZ};
}

}  // namespace

auto make_general_plane_reflect_filter_type(const input_parser::general_plane_reflect_filter& aInput,
                                            const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
    -> GeneralPlaneReflectFilterType
{
    const auto tFilterRadius = extension::detail::get_filter_radius(aInput, aAnalysisDomainMesh.mFileName);
    const auto tGeneralPlane = Plane(center_coordinate(aInput), plane_normal(aInput));
    return detail::make_general_plane_reflect_filter_type(tFilterRadius, aInput.centering_type.value(), tGeneralPlane);
}

namespace detail
{
auto make_general_plane_reflect_filter_type(double aRadius,
                                            input_parser::KernelFilterCenteringTypes aFilterCentering,
                                            const Plane& aPlane) -> GeneralPlaneReflectFilterType
{
    const auto tSearchFunction =
        [aRadius, tPlane = aPlane](
            const TargetRowVector& aTargetRowVector, const SourceColumnVector& aSourceColumnVector,
            const boost::mpi::communicator& aLambdaCommunicator) -> third_party_integration::stk_search::SearchResults
    {
        return distribute_search_vectors_and_stk_search_with_spheres(aTargetRowVector, aSourceColumnVector, aRadius,
                                                                     aLambdaCommunicator, tPlane);
    };

    const auto tUnnormalizedWeightFunction = [aRadius, tPlane = aPlane](const SourcePoint& aSourcePoint,
                                                                        const TargetPoint& aTargetPoint) -> double
    {
        const double tDistance = minimum_distance_reflected_points_planes_list(aSourcePoint, aTargetPoint, tPlane);
        return detail::linear_ramp_weight(Distance{tDistance}, SearchRadius{aRadius});
    };

    const auto tTargetMeshFunction = [](const input_parser::general_plane_reflect_filter& aInput) -> mesh::Mesh
    {
        const auto tFixedBlocks = mesh::fixed_blocks(aInput);
        return mesh::Mesh{aInput.target_mesh_name.value().mToken, tFixedBlocks};
    };

    return GeneralPlaneReflectFilterType{
        .mKernelFilterFunctions =
            KernelFilterFunctions{
                .mRadius = aRadius,
                .mFilterCentering = aFilterCentering,
                .mSearchFunction = tSearchFunction,
                .mUnnormalizedWeightFunction = tUnnormalizedWeightFunction,
                .mLoggingName = input_parser::block_name<input_parser::general_plane_reflect_filter>()},
        .mTargetMeshFunction = tTargetMeshFunction};
}

auto validate_source_mesh_on_outward_normal_side_of_plane(const input_parser::general_plane_reflect_filter& aInput,
                                                          const std::filesystem::path& aSourceMeshFileName)
    -> std::optional<std::string>
{
    namespace tpic = third_party_integration::common;
    if (!std::filesystem::exists(aSourceMeshFileName) ||
        validate_required_plane_normal_is_non_zero_magnitude(aInput).has_value())
    {
        return std::nullopt;  // Not our error
    }
    const auto tSourceCoordinates = mesh::EntityRetrieval{mesh::Mesh{aSourceMeshFileName}}.nodalCoordinates();
    const auto tPlane = Plane(center_coordinate(aInput), plane_normal(aInput));

    const auto tPredicate = [tPlane](const tpic::Coordinate& aPoint)
    { return signed_distance_to_plane(aPoint, tPlane) <= -reliable_mesh_zero_value(); };

    auto tOutOfRangeCoordinates = utilities::reserved_container<std::vector<std::string>>(tSourceCoordinates.size());
    utilities::transform_if(
        tSourceCoordinates, std::back_inserter(tOutOfRangeCoordinates),
        [](const tpic::Coordinate& aPoint) { return tpic::string(aPoint, kValidationOutputPrecision); }, tPredicate);

    if (!tOutOfRangeCoordinates.empty())
    {
        const auto tNumberOfViolatingPoints = tOutOfRangeCoordinates.size();
        constexpr auto tMaxPrintSize = std::size_t{10};
        if (tNumberOfViolatingPoints > tMaxPrintSize)
        {
            tOutOfRangeCoordinates.resize(tMaxPrintSize);
        }
        const auto tListOfPoints = utilities::concatenate_container(tOutOfRangeCoordinates);
        return std::optional<std::string>{utilities::concatenate(
            input_parser::block_name<input_parser::general_plane_reflect_filter>(),
            ": source mesh must be on the positive side of the plane. Defined plane had center ",
            tpic::string(tPlane.mCenter, kValidationOutputPrecision), " and normal ",
            tpic::string(static_cast<tpic::Vector3>(tPlane.mUnitNormal), kValidationOutputPrecision), ". Found ",
            tNumberOfViolatingPoints, " points in violation. The first ", tOutOfRangeCoordinates.size(),
            " point(s) are: ", tListOfPoints, ".")};
    }
    return std::nullopt;
}

auto validate_required_plane_normal_is_non_zero_magnitude(const input_parser::general_plane_reflect_filter& aInput)
    -> std::optional<std::string>
{
    if (aInput.plane.has_value())
    {
        const auto tNormalPoint = aInput.plane.value().normal;
        if (const auto tNormalMagnitude = third_party_integration::common::magnitude(
                third_party_integration::common::Coordinate{tNormalPoint.mX, tNormalPoint.mY, tNormalPoint.mZ});
            tNormalMagnitude < std::numeric_limits<double>::min())
        {
            return utilities::concatenate(input_parser::block_name<input_parser::general_plane_reflect_filter>(),
                                          ": a plane must have a nonzero normal, found a normal with magnitude ",
                                          tNormalMagnitude, ".");
        }
    }
    return std::nullopt;
}

auto validate_required_plane(const input_parser::general_plane_reflect_filter& aInput) -> std::optional<std::string>
{
    return input_validation::error_message_for_empty_parameter(
        input_parser::block_name<input_parser::general_plane_reflect_filter>(), aInput.plane,
        "general_plane_reflect_filter");
}

}  // namespace detail

}  // namespace plato::filter::extension::kernel_filters
