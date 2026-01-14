#include "plato/filter/extension/kernel_filters/SweptFilter.hpp"

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
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/mesh/FixedBlockUtilities.hpp"

namespace plato::filter::extension::kernel_filters
{

namespace
{
const auto kMakeSweptFilterInfo = [](const input_parser::z_swept_filter& aInput,
                                     const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> SweptFilterType
{ return make_z_swept_filter_type(aInput, aAnalysisDomainMesh); };

[[maybe_unused]] static auto kSweptFilterParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::z_swept_filter>{};

[[maybe_unused]] static auto kSweptFilterRegistration = library::FilterRegistration{
    input_parser::block_name<input_parser::z_swept_filter>(), [](const library::ValidatedFilterInput& aInput)
    {
        const auto& tInput = input_validation::get_input_block<input_parser::z_swept_filter>(aInput);
        return library::make_filter_function_from_cache(
            [&tInput]()
            { return detail::create_kernel_filter_cache<input_parser::z_swept_filter>(tInput, kMakeSweptFilterInfo); });
    }};

[[maybe_unused]] static auto kSweptFilterValidationRegistration =
    validation_for_all_kernel_filters<input_parser::z_swept_filter>();

[[maybe_unused]] static auto kSweptFilterTargetMeshValidationRegistration =
    target_mesh_validation_for_all_kernel_filters<input_parser::z_swept_filter>();

[[maybe_unused]] static auto kSweptFilterMeshBasedValidationRegistration =
    input_validation::InputBlockValidationRegistration<std::filesystem::path>{
        [](const input_parser::z_swept_filter& aInput, const std::filesystem::path& aMeshPath)
        { return extension::detail::validate_filter_radius_with_mesh(aInput, aMeshPath); },
        [](const input_parser::z_swept_filter& aInput, const std::filesystem::path& aMeshPath)
        { return detail::validate_source_mesh_in_xy_plane(aInput, aMeshPath); },
        [](const input_parser::z_swept_filter& aInput, const std::filesystem::path& aMeshPath)
        {
            return validate_all_target_domain_find_source_domain<input_parser::z_swept_filter>(
                aInput, aMeshPath, kMakeSweptFilterInfo, boost::mpi::communicator{});
        }};

}  // namespace

auto make_z_swept_filter_type(const input_parser::z_swept_filter& aInput,
                              const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> SweptFilterType
{
    const auto tFilterRadius = extension::detail::get_filter_radius(aInput, aAnalysisDomainMesh.mFileName);
    const auto tZHat = third_party_integration::common::Vector3{0, 0, 1};
    return detail::make_z_swept_filter_type(tFilterRadius, aInput.centering_type.value(), tZHat);
}

namespace detail
{
auto make_z_swept_filter_type(const double aRadius,
                              const input_parser::KernelFilterCenteringTypes aFilterCentering,
                              const third_party_integration::common::Vector3& aDirection) -> SweptFilterType
{
    const auto tSearchFunction =
        [aRadius, aDirection](
            const TargetRowVector& aTargetRowVector, const SourceColumnVector& aSourceColumnVector,
            const boost::mpi::communicator& aLambdaCommunicator) -> third_party_integration::stk_search::SearchResults
    {
        const auto tBoundingBox = third_party_integration::common::bounding_box(
            third_party_integration::common::bounding_box(aTargetRowVector.mValue),
            third_party_integration::common::bounding_box(aSourceColumnVector.mValue));

        const auto tDimension =
            third_party_integration::common::dot(tBoundingBox.second - tBoundingBox.first, aDirection);
        return detail::distribute_search_vectors_and_stk_search_with_cylinders(
            aTargetRowVector, aSourceColumnVector, CylinderAxis{aDirection * tDimension}, CylinderRadius{aRadius},
            aLambdaCommunicator);
    };

    const auto tUnnormalizedWeightFunction = [aRadius, aDirection](const SourcePoint& aSourcePoint,
                                                                   const TargetPoint& aTargetPoint) -> double
    {
        return detail::cylinder_ramp_weight(detail::CylinderCenter{aTargetPoint.mValue}, CylinderAxis{aDirection},
                                            CylinderRadius{aRadius}, detail::QueryLocation{aSourcePoint.mValue});
    };

    const auto tTargetMeshFunction = [](const input_parser::z_swept_filter& aInput) -> mesh::Mesh
    {
        const auto tFixedBlocks = mesh::fixed_blocks(aInput);
        return mesh::Mesh{aInput.target_mesh_name.value().mToken, tFixedBlocks};
    };

    return SweptFilterType{aRadius, aFilterCentering, tSearchFunction, tUnnormalizedWeightFunction,
                           tTargetMeshFunction};
}

auto validate_source_mesh_in_xy_plane(const input_parser::z_swept_filter& /*aInput*/,
                                      const std::filesystem::path& aSourceMeshFileName) -> std::optional<std::string>
{
    if (!std::filesystem::exists(aSourceMeshFileName))
    {
        return std::nullopt;  // Not our error
    }
    const auto tBoundingBox = mesh::EntityRetrieval{mesh::Mesh{aSourceMeshFileName}}.designDomainBoundingBox();
    if (tBoundingBox.first.z != tBoundingBox.second.z)
    {
        return std::optional<std::string>{utilities::concatenate(
            input_parser::block_name<input_parser::z_swept_filter>(), ": source mesh must exist in the XY plane.")};
    }
    return std::nullopt;
}

}  // namespace detail

}  // namespace plato::filter::extension::kernel_filters
