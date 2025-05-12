#ifndef PLATO_PROCESS_MANAGER_EXTENSION_ELEMENTTONODERESULTFILTER
#define PLATO_PROCESS_MANAGER_EXTENSION_ELEMENTTONODERESULTFILTER

#include "plato/input_parser/CrossReference.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"

// clang-format off
PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), nodal_result_filter,
    (plato::input_parser::CrossReference<plato::input_parser::ComponentType::kFilter>, filter, "Name of the filter to apply. "
                                                                                               "Only required if more than one filter is specified.")
    (plato::input_parser::CrossReference<plato::input_parser::ComponentType::kGeometry>, geometry, "Name of the geometry whose output mesh will be used. "
                                                                                                   "Only required if more than one geometry is specified.")
    (plato::input_parser::FileName, output_file_name, "Optional: If omitted, the output file from density_topology will be used and overwritten.")
)
// clang-format on

namespace plato::process_manager::library
{
struct ProcessManagerData;
}

namespace plato::process_manager::extension
{

/// @brief The purpose of this process manager is to convert an element-based filter result from a density topology
/// run to a nodal result.
///
/// Element-based fields can be difficult to post-process in some visualization software and so this enables users
/// to generate a nodal field result if desired.
class NodalResultFilter
{
   public:
    explicit NodalResultFilter(const library::ValidatedProcessManagerInput& aInput);

    void run() const;

    /// @brief Returns the name of the filtered field that is inserted into the output mesh.
    [[nodiscard]] static auto field_name() -> std::string_view;

   private:
    std::filesystem::path mInputMeshPath;
    std::filesystem::path mOutputMeshPath;
    std::set<std::string> mFixedBlockNames;
    double mFilterRadius;
    unsigned int mNumberOfProcessorsForFilter;
};

namespace detail
{
/// @brief Checks that the cross-linked filter in @a aInput is a kernel_filter.
[[nodiscard]] auto validate_filter_is_kernel_filter(const input_parser::nodal_result_filter& aInput)
    -> std::optional<std::string>;

/// @brief Checks that the cross-linked geometry in @a aInput is a density_topology.
[[nodiscard]] auto validate_geometry_is_density_topology(const input_parser::nodal_result_filter& aInput)
    -> std::optional<std::string>;

/// @brief Checks that the cross-reference @a aCrossReference is the expected type, given by @a ExpectedCrossReference.
template <typename ExpectedCrossReference, input_parser::ComponentType kComponentType>
[[nodiscard]] auto validate_expected_cross_reference_type(
    const input_parser::CrossReference<kComponentType>& aCrossReference) -> std::optional<std::string>;

template <typename ExpectedCrossReference, input_parser::ComponentType kComponentType>
auto validate_expected_cross_reference_type(
    const boost::optional<input_parser::CrossReference<kComponentType>>& aCrossReference) -> std::optional<std::string>
{
    if (aCrossReference && !aCrossReference->mInputBlock.template holdsExpectedType<ExpectedCrossReference>())
    {
        return std::optional<std::string>{
            input_parser::block_name<input_parser::nodal_result_filter>() + " requires that a " +
            input_parser::block_name<ExpectedCrossReference>() +
            " component be used. The incorrect component type is: " + aCrossReference->mName};
    }
    return {};
}

}  // namespace detail

}  // namespace plato::process_manager::extension

#endif
