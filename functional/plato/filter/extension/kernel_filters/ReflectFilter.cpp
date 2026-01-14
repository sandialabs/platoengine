#include "plato/filter/extension/kernel_filters/ReflectFilter.hpp"

#include <boost/mpi.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/vector.hpp>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/filter/extension/CommonInputValidation.hpp"
#include "plato/filter/extension/FilterMeshUtilities.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilter.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilterInputUtilities.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilder.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilderUtilities.hpp"
#include "plato/filter/extension/kernel_filters/ReflectionUtilities.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/mesh/FixedBlockUtilities.hpp"

namespace plato::filter::extension::kernel_filters
{

namespace
{
const auto kMakeReflectFilterInfo = [](const input_parser::positive_octant_reflect_filter& aInput,
                                       const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> ReflectFilterType
{ return make_positive_octant_reflect_filter_type(aInput, aAnalysisDomainMesh); };

[[maybe_unused]] static auto kReflectFilterParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::positive_octant_reflect_filter>{};

[[maybe_unused]] static auto kReflectFilterRegistration = library::FilterRegistration{
    input_parser::block_name<input_parser::positive_octant_reflect_filter>(),
    [](const library::ValidatedFilterInput& aInput)
    {
        const auto& tInput = input_validation::get_input_block<input_parser::positive_octant_reflect_filter>(aInput);
        return library::make_filter_function_from_cache(
            [&tInput]()
            {
                return detail::create_kernel_filter_cache<input_parser::positive_octant_reflect_filter>(
                    tInput, kMakeReflectFilterInfo);
            });
    }};

[[maybe_unused]] static auto kReflectFilterValidationRegistration =
    validation_for_all_kernel_filters<input_parser::positive_octant_reflect_filter>();

[[maybe_unused]] static auto kReflectFilterTargetMeshValidationRegistration =
    target_mesh_validation_for_all_kernel_filters<input_parser::positive_octant_reflect_filter>();

[[maybe_unused]] static auto kReflectFilterMeshBasedValidationRegistration =
    input_validation::InputBlockValidationRegistration<std::filesystem::path>{
        [](const input_parser::positive_octant_reflect_filter& aInput, const std::filesystem::path& aMeshPath)
        { return extension::detail::validate_filter_radius_with_mesh(aInput, aMeshPath); },
        [](const input_parser::positive_octant_reflect_filter& aInput, const std::filesystem::path& aMeshPath)
        { return detail::validate_source_mesh_in_positive_octant(aInput, aMeshPath); },
        [](const input_parser::positive_octant_reflect_filter& aInput, const std::filesystem::path& aMeshPath)
        {
            return validate_all_target_domain_find_source_domain<input_parser::positive_octant_reflect_filter>(
                aInput, aMeshPath, kMakeReflectFilterInfo, boost::mpi::communicator{});
        }};

}  // namespace

auto make_positive_octant_reflect_filter_type(const input_parser::positive_octant_reflect_filter& aInput,
                                              const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
    -> ReflectFilterType
{
    const auto tFilterRadius = extension::detail::get_filter_radius(aInput, aAnalysisDomainMesh.mFileName);
    return detail::make_positive_octant_reflect_filter_type(tFilterRadius, aInput.centering_type.value());
}

namespace detail
{
auto make_positive_octant_reflect_filter_type(const double aRadius,
                                              const input_parser::KernelFilterCenteringTypes aFilterCentering)
    -> ReflectFilterType
{
    const auto tPositiveXPlane = Plane({0, 0, 0}, {1, 0, 0});
    const auto tPositiveYPlane = Plane({0, 0, 0}, {0, 1, 0});
    const auto tPositiveZPlane = Plane({0, 0, 0}, {0, 0, 1});
    const auto tSearchFunction =
        [aRadius, tPositiveXPlane, tPositiveYPlane, tPositiveZPlane](
            const TargetRowVector& aTargetRowVector, const SourceColumnVector& aSourceColumnVector,
            const boost::mpi::communicator& aLambdaCommunicator) -> third_party_integration::stk_search::SearchResults
    {
        return distribute_search_vectors_and_stk_search_with_spheres(aTargetRowVector, aSourceColumnVector, aRadius,
                                                                     aLambdaCommunicator, tPositiveXPlane,
                                                                     tPositiveYPlane, tPositiveZPlane);
    };

    const auto tUnnormalizedWeightFunction = [aRadius, tPositiveXPlane, tPositiveYPlane, tPositiveZPlane](
                                                 const SourcePoint& aSourcePoint,
                                                 const TargetPoint& aTargetPoint) -> double
    {
        const double tDistance = minimum_distance_reflected_points_planes_list(
            aSourcePoint, aTargetPoint, tPositiveXPlane, tPositiveYPlane, tPositiveZPlane);
        return detail::linear_ramp_weight(Distance{tDistance}, SearchRadius{aRadius});
    };

    const auto tTargetMeshFunction = [](const input_parser::positive_octant_reflect_filter& aInput) -> mesh::Mesh
    {
        const auto tFixedBlocks = mesh::fixed_blocks(aInput);
        return mesh::Mesh{aInput.target_mesh_name.value().mToken, tFixedBlocks};
    };

    return ReflectFilterType{aRadius, aFilterCentering, tSearchFunction, tUnnormalizedWeightFunction,
                             tTargetMeshFunction};
}

auto validate_source_mesh_in_positive_octant(const input_parser::positive_octant_reflect_filter& /*aInput*/,
                                             const std::filesystem::path& aSourceMeshFileName)
    -> std::optional<std::string>
{
    if (!std::filesystem::exists(aSourceMeshFileName))
    {
        return std::nullopt;  // Not our error
    }
    const auto tBoundingBox = mesh::EntityRetrieval{mesh::Mesh{aSourceMeshFileName}}.designDomainBoundingBox();

    if (tBoundingBox.first.x != 0 || tBoundingBox.first.y != 0 || tBoundingBox.first.z != 0)
    {
        return std::optional<std::string>{
            utilities::concatenate(input_parser::block_name<input_parser::positive_octant_reflect_filter>(),
                                   ": source mesh must be in the positive octant. Found this as the lower bounds: ",
                                   std::to_string(tBoundingBox.first.x), " ", std::to_string(tBoundingBox.first.y), " ",
                                   std::to_string(tBoundingBox.first.z), ".")};
    }
    return std::nullopt;
}

}  // namespace detail

}  // namespace plato::filter::extension::kernel_filters
