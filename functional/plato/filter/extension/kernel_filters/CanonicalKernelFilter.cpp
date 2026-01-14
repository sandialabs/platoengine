#include "plato/filter/extension/kernel_filters/CanonicalKernelFilter.hpp"

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
#include "plato/filter/extension/kernel_filters/SphericalMaskUtilities.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"

namespace plato::filter::extension::kernel_filters
{

namespace
{

const auto kKernelFilterLambda = [](const input_parser::kernel_filter& aInput,
                                    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> KernelFilterType
{ return make_kernel_filter_type(aInput, aAnalysisDomainMesh); };

[[maybe_unused]] static auto kKernelFilterParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::kernel_filter>{};

[[maybe_unused]] static auto kKernelFilterRegistration = library::FilterRegistration{
    input_parser::block_name<input_parser::kernel_filter>(), [](const library::ValidatedFilterInput& aInput)
    {
        const auto& tInput = input_validation::get_input_block<input_parser::kernel_filter>(aInput);
        return library::make_filter_function_from_cache(
            [&tInput]()
            { return detail::create_kernel_filter_cache<input_parser::kernel_filter>(tInput, kKernelFilterLambda); });
    }};

[[maybe_unused]] static auto kKernelFilterValidationRegistration =
    validation_for_all_kernel_filters<input_parser::kernel_filter>();

[[maybe_unused]] static auto kKernelFilterMeshBasedValidationRegistration =
    input_validation::InputBlockValidationRegistration<std::filesystem::path>{
        [](const input_parser::kernel_filter& aInput, const std::filesystem::path& aMeshPath)
        { return extension::detail::validate_filter_radius_with_mesh(aInput, aMeshPath); }};

}  // namespace

auto make_kernel_filter_type(const input_parser::kernel_filter& aInput,
                             const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> KernelFilterType
{
    const auto tFilterRadius = extension::detail::get_filter_radius(aInput, aAnalysisDomainMesh.mFileName);
    return detail::make_kernel_filter_type(tFilterRadius, aInput.centering_type.value(),
                                           mesh::Mesh{aAnalysisDomainMesh});
}

namespace detail
{
auto make_kernel_filter_type(const double aRadius,
                             const input_parser::KernelFilterCenteringTypes aFilterCentering,
                             const mesh::Mesh& aTargetMesh) -> KernelFilterType
{
    const auto tSearchFunction =
        [aRadius](
            const TargetRowVector& aTargetRowVector, const SourceColumnVector& aSourceColumnVector,
            const boost::mpi::communicator& aLambdaCommunicator) -> third_party_integration::stk_search::SearchResults
    {
        return distribute_search_vectors_and_stk_search_with_spheres(aTargetRowVector, aSourceColumnVector, aRadius,
                                                                     aLambdaCommunicator);
    };

    const auto tUnnormalizedWeightFunction = [aRadius](const SourcePoint& aSourcePoint,
                                                       const TargetPoint& aTargetPoint) -> double
    {
        const double tDistance = third_party_integration::common::magnitude(aSourcePoint.mValue - aTargetPoint.mValue);
        return detail::linear_ramp_weight(Distance{tDistance}, SearchRadius{aRadius});
    };

    const auto tTargetMeshFunction = [aTargetMesh](const input_parser::kernel_filter&) -> mesh::Mesh
    { return aTargetMesh; };

    return KernelFilterType{aRadius, aFilterCentering, tSearchFunction, tUnnormalizedWeightFunction,
                            tTargetMeshFunction};
}

}  // namespace detail

}  // namespace plato::filter::extension::kernel_filters
