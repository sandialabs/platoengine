#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_KERNELFILTERINPUTUTILITIES
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_KERNELFILTERINPUTUTILITIES

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/filter/extension/CommonInputValidation.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilder.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/FixedBlockUtilities.hpp"

namespace plato::filter::extension::kernel_filters
{

constexpr inline auto kKernelFilterRadiusHelpText = "Required field specifying the size of the filter radius.";

constexpr inline auto kKernelFilterCenteringTypeHelpText =
    "Required field specifying whether the results should be 'node' or 'element' centered. "
    "Platoanalyze requires node-centered and SD requires element-centered.";

constexpr inline auto kKernelFilterRelativeRadiusHelpText =
    "Optional field that can convert the filter radius specified into a radius relative to the average element size.";

constexpr inline auto kKernelFilterNumberOfProcessorsHelpText =
    "Optional parameter that will specify the number of processors used to run the filter in parallel. "
    "Requires the 'plato' executable be called with mpirun. Limited to the maximum processors specified in the mpirun "
    "call. "
    "Over-parallelization can cause a significant degredation of the filter.";

constexpr inline auto kKernelFilterFixedBlockHelpText =
    "Optional field that specifies the fixed blocks in the target mesh.";

///@brief Validation test to verify target_mesh is defined and exists on disk.
template <typename InputParserType>
[[nodiscard]] auto validate_target_mesh_exists(const InputParserType& aInput) -> std::optional<std::string>;

///@brief Validation test to verify target_mesh is compatible with defined radius.
template <typename InputParserType>
[[nodiscard]] auto validate_target_mesh_with_filter_radius(const InputParserType& aInput) -> std::optional<std::string>;

///@brief Create a InputBlockValidationRegistration that applies to all kernel_filter types
template <typename InputParserType>
[[nodiscard]] auto validation_for_all_kernel_filters() -> input_validation::InputBlockValidationRegistration<>;

///@brief Create a InputBlockValidationRegistration that applies to all kernel_filter types with target meshes
template <typename InputParserType>
[[nodiscard]] auto target_mesh_validation_for_all_kernel_filters()
    -> input_validation::InputBlockValidationRegistration<>;

///@brief Validation function that ensures that all points in the target domain map to some entity in the source, i.e.,
/// there are no zero rows in the matrix.
template <typename InputParserType>
[[nodiscard]] auto validate_all_target_domain_find_source_domain(const InputParserType& aInput,
                                                                 const std::filesystem::path& aSourceMeshFileName,
                                                                 const auto& aMakeFunctionLambda,
                                                                 const boost::mpi::communicator& aCommunicator)
    -> std::optional<std::string>;

namespace detail
{
///@brief Function that checks the search results @a aSearchResults to make sure it has all of the ids up to @a aSize
[[nodiscard]] auto search_results_contain_all_row_ids(
    const third_party_integration::stk_search::SearchResults& aSearchResults, const std::size_t aSize) -> bool;
}  // namespace detail

template <typename InputParserType>
auto validation_for_all_kernel_filters() -> input_validation::InputBlockValidationRegistration<>
{
    return input_validation::InputBlockValidationRegistration<>{
        [](const InputParserType& aInput) { return extension::detail::validate_filter_radius_bounds(aInput); },
        [](const InputParserType& aInput) { return extension::detail::validate_filter_centering_type(aInput); },
        [](const InputParserType& aInput) { return extension::detail::validate_number_of_processors(aInput); },
        [](const InputParserType& aInput)
        { return extension::detail::validate_number_of_processors_factor_of_comm_world(aInput); }};
}

template <typename InputParserType>
auto target_mesh_validation_for_all_kernel_filters() -> input_validation::InputBlockValidationRegistration<>
{
    constexpr auto tMeshNameAccessor = [](const auto& aInput) { return aInput.target_mesh_name; };
    return input_validation::InputBlockValidationRegistration<>{
        [](const InputParserType& aInput) { return validate_target_mesh_exists<InputParserType>(aInput); },
        [](const InputParserType& aInput) { return validate_target_mesh_with_filter_radius<InputParserType>(aInput); },
        [tMeshNameAccessor](const InputParserType& aInput)
        { return mesh::validate_unique_fixed_block_names(aInput, tMeshNameAccessor); },
        [tMeshNameAccessor](const InputParserType& aInput)
        { return mesh::validate_fixed_block_names_exist(aInput, tMeshNameAccessor); },
        [tMeshNameAccessor](const InputParserType& aInput)
        { return mesh::validate_at_least_one_design_block(aInput, tMeshNameAccessor); }};
}

template <typename InputParserType>
auto validate_target_mesh_exists(const InputParserType& aInput) -> std::optional<std::string>
{
    constexpr auto tTargetMeshNameAccessor = [](const auto& aFilterInput) { return aFilterInput.target_mesh_name; };
    return input_validation::error_message_for_missing_file_on_disk(aInput, tTargetMeshNameAccessor,
                                                                    "target_mesh_name");
}

template <typename InputParserType>
auto validate_target_mesh_with_filter_radius(const InputParserType& aInput) -> std::optional<std::string>
{
    if (aInput.target_mesh_name)
    {
        return extension::detail::validate_filter_radius_with_mesh<InputParserType>(
            aInput, aInput.target_mesh_name.value().mToken);
    }
    else
    {
        return std::nullopt;  // not our error
    }
}

template <typename InputParserType>
[[nodiscard]] auto validate_all_target_domain_find_source_domain(const InputParserType& aInput,
                                                                 const std::filesystem::path& aSourceMeshFileName,
                                                                 const auto& aMakeFunctionLambda,
                                                                 const boost::mpi::communicator& aCommunicator)
    -> std::optional<std::string>
{
    const auto& tTargetMeshName = aInput.target_mesh_name;
    if (tTargetMeshName.has_value() && std::filesystem::exists(tTargetMeshName.value().mToken) &&
        std::filesystem::exists(aSourceMeshFileName))
    {
        const auto tFilterType = aMakeFunctionLambda(aInput, analysis::AnalysisDomainMesh{aSourceMeshFileName, {}});
        const auto tSourceCoordinates =
            mesh::EntityRetrieval{mesh::Mesh{aSourceMeshFileName}}.designDomainNodalCoordinates();
        const auto tTargetCoordinates = detail::center_coordinates(
            mesh::Mesh{tTargetMeshName.value().mToken, mesh::fixed_blocks(aInput)}, tFilterType.mFilterCentering);
        const auto tSearchResults = tFilterType.mSearchFunction(TargetRowVector{tTargetCoordinates},
                                                                SourceColumnVector{tSourceCoordinates}, aCommunicator);

        const bool tAllFound = detail::search_results_contain_all_row_ids(tSearchResults, tTargetCoordinates.size());

        if (!tAllFound)
        {
            return utilities::concatenate(input_parser::block_name<InputParserType>(),
                                          ": Target mesh nodes or centroids are not properly mapped to a source mesh "
                                          "node under the symmetry operation requested.",
                                          "Check 'fixed_blocks' specification for both source and target mesh in the "
                                          "geometry specification and symmetry filter, respectively.");
        }
    }

    return std::nullopt;
}

}  // namespace plato::filter::extension::kernel_filters

#endif
