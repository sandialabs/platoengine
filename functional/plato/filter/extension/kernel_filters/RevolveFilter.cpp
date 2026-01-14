#include "plato/filter/extension/kernel_filters/RevolveFilter.hpp"

#include <boost/mpi.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/vector.hpp>
#include <optional>

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
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/FixedBlockUtilities.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::filter::extension::kernel_filters
{

namespace
{
const auto kMakeRevolveFilterInfo = [](const input_parser::y_axis_revolve_filter& aInput,
                                       const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> RevolveFilterType
{ return make_y_axis_revolve_filter_type(aInput, aAnalysisDomainMesh); };

[[maybe_unused]] static auto kRevolveFilterParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::y_axis_revolve_filter>{};

[[maybe_unused]] static auto kRevolveFilterRegistration = library::FilterRegistration{
    input_parser::block_name<input_parser::y_axis_revolve_filter>(), [](const library::ValidatedFilterInput& aInput)
    {
        const auto& tInput = input_validation::get_input_block<input_parser::y_axis_revolve_filter>(aInput);
        return library::make_filter_function_from_cache(
            [&tInput]()
            {
                return detail::create_kernel_filter_cache<input_parser::y_axis_revolve_filter>(tInput,
                                                                                               kMakeRevolveFilterInfo);
            });
    }};

[[maybe_unused]] static auto kRevolveFilterValidationRegistration =
    validation_for_all_kernel_filters<input_parser::y_axis_revolve_filter>();

[[maybe_unused]] static auto kRevolveFilterTargetMeshValidationRegistration =
    target_mesh_validation_for_all_kernel_filters<input_parser::y_axis_revolve_filter>();

[[maybe_unused]] static auto kRevolveFilterMeshBasedValidationRegistration =
    input_validation::InputBlockValidationRegistration<std::filesystem::path>{
        [](const input_parser::y_axis_revolve_filter& aInput, const std::filesystem::path& aMeshPath)
        { return extension::detail::validate_filter_radius_with_mesh(aInput, aMeshPath); },
        [](const input_parser::y_axis_revolve_filter& aInput, const std::filesystem::path& aMeshPath)
        { return detail::validate_source_mesh_in_xz_plane(aInput, aMeshPath); },
        [](const input_parser::y_axis_revolve_filter& aInput, const std::filesystem::path& aMeshPath)
        {
            return validate_all_target_domain_find_source_domain<input_parser::y_axis_revolve_filter>(
                aInput, aMeshPath, kMakeRevolveFilterInfo, boost::mpi::communicator{});
        }};

}  // namespace

auto make_y_axis_revolve_filter_type(const input_parser::y_axis_revolve_filter& aInput,
                                     const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> RevolveFilterType
{
    const auto tFilterRadius = extension::detail::get_filter_radius(aInput, aAnalysisDomainMesh.mFileName);
    return detail::make_y_axis_revolve_filter_type(tFilterRadius, aInput.centering_type.value());
}

namespace detail
{
auto make_y_axis_revolve_filter_type(const double aRadius,
                                     const input_parser::KernelFilterCenteringTypes aFilterCentering)
    -> RevolveFilterType
{
    const auto tThetaHat = third_party_integration::common::Vector3{0, 1, 0};
    const auto tSearchFunction =
        [aRadius, tThetaHat](
            const TargetRowVector& aTargetRowVector, const SourceColumnVector& aSourceColumnVector,
            const boost::mpi::communicator& aLambdaCommunicator) -> third_party_integration::stk_search::SearchResults
    {
        const auto tCylindricalPolarSources =
            SourceColumnVector{detail::cylindrical_polar_coordinates(aSourceColumnVector.mValue).mValue};
        const auto tCylindricalPolarTargets =
            TargetRowVector{detail::cylindrical_polar_coordinates(aTargetRowVector.mValue).mValue};
        const auto tDimension = double{2 * std::numbers::pi};
        return detail::distribute_search_vectors_and_stk_search_with_cylinders(
            tCylindricalPolarTargets, tCylindricalPolarSources, CylinderAxis{tThetaHat * tDimension},
            CylinderRadius{aRadius}, aLambdaCommunicator);
    };

    const auto tUnnormalizedWeightFunction = [aRadius, tThetaHat](const SourcePoint& aSourcePoint,
                                                                  const TargetPoint& aTargetPoint) -> double
    {
        const auto tCylindricalPolarSourcePoint = detail::cylindrical_polar_coordinates(aSourcePoint.mValue).mValue;
        const auto tCylindricalPolarTargetPoint = detail::cylindrical_polar_coordinates(aTargetPoint.mValue).mValue;

        return detail::cylinder_ramp_weight(detail::CylinderCenter{tCylindricalPolarTargetPoint},
                                            CylinderAxis{tThetaHat}, CylinderRadius{aRadius},
                                            detail::QueryLocation{tCylindricalPolarSourcePoint});
    };

    const auto tTargetMeshFunction = [](const input_parser::y_axis_revolve_filter& aInput) -> mesh::Mesh
    {
        const auto tFixedBlocks = mesh::fixed_blocks(aInput);
        return mesh::Mesh{aInput.target_mesh_name.value().mToken, tFixedBlocks};
    };

    return RevolveFilterType{aRadius, aFilterCentering, tSearchFunction, tUnnormalizedWeightFunction,
                             tTargetMeshFunction};
}

auto validate_source_mesh_in_xz_plane(const input_parser::y_axis_revolve_filter& /*aInput*/,
                                      const std::filesystem::path& aSourceMeshFileName) -> std::optional<std::string>
{
    if (!std::filesystem::exists(aSourceMeshFileName))
    {
        return std::nullopt;  // Not our error
    }
    const auto tBoundingBox = mesh::EntityRetrieval{mesh::Mesh{aSourceMeshFileName}}.designDomainBoundingBox();
    const auto tDimensions = tBoundingBox.second - tBoundingBox.first;
    if (tDimensions.y != 0 || tDimensions.x == 0 || tDimensions.z == 0)
    {
        return std::optional<std::string>{utilities::concatenate(
            input_parser::block_name<input_parser::y_axis_revolve_filter>(),
            ": source mesh must exist only in the XZ plane. Your mesh has dimensions: ", std::to_string(tDimensions.x),
            ", ", std::to_string(tDimensions.y), ", ", std::to_string(tDimensions.z))};
    }
    return std::nullopt;
}
}  // namespace detail

}  // namespace plato::filter::extension::kernel_filters
