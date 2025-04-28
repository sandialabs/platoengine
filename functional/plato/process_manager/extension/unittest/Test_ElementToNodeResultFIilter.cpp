#include <gtest/gtest.h>

#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/filter/extension/KernelFilter.hpp"
#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/process_manager/extension/ElementToNodeResultFilter.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"

namespace plato::process_manager::extension::unittest
{
namespace
{
template <typename ComponentInput>
[[nodiscard]] auto cross_reference()
{
    return input_parser::CrossReference<input_parser::ComponentTypeOfInputBlock<ComponentInput>::value>{
        input_parser::block_name<ComponentInput>(), input_parser::InputBlockWrapper{ComponentInput{}}};
}
}  // namespace

TEST(ElementToNodeResultFilter, CreateAndRun) {}

TEST(ElementToNodeResultFilter, ValidateFilterIsKernelFilter)
{
    {
        // Wrong filter
        auto tInput = input_parser::element_to_node_result_filter{};
        tInput.filter = cross_reference<input_parser::identity_filter>();
        EXPECT_TRUE(detail::validate_filter_is_kernel_filter(tInput));
    }
    {
        // Correct filter
        auto tInput = input_parser::element_to_node_result_filter{};
        tInput.filter = cross_reference<input_parser::kernel_filter>();
        const auto tErrorMessage = detail::validate_filter_is_kernel_filter(tInput);
        EXPECT_FALSE(tErrorMessage) << tErrorMessage.value();
    }
    // Via registered validation
}

TEST(ElementToNodeResultFilter, ValidateHasMeshName)
{
    {
        // Has no mesh name and wrong geometry
        auto tInput = input_parser::element_to_node_result_filter{};
        tInput.geometry = cross_reference<input_parser::brick_shape_geometry>();
        EXPECT_TRUE(detail::validate_has_mesh_name_or_geometry_is_density_topology(tInput));
    }
    {
        // Has no mesh name and correct geometry
        auto tInput = input_parser::element_to_node_result_filter{};
        tInput.geometry = cross_reference<input_parser::density_topology>();
        const auto tErrorMessage = detail::validate_has_mesh_name_or_geometry_is_density_topology(tInput);
        EXPECT_FALSE(detail::validate_has_mesh_name_or_geometry_is_density_topology(tInput)) << tErrorMessage.value();
    }
    {
        // Has mesh name and wrong geometry
        auto tInput = input_parser::element_to_node_result_filter{};
        tInput.geometry = cross_reference<input_parser::brick_shape_geometry>();
        tInput.output_file_name = input_parser::FileName{};
        const auto tErrorMessage = detail::validate_has_mesh_name_or_geometry_is_density_topology(tInput);
        EXPECT_FALSE(detail::validate_has_mesh_name_or_geometry_is_density_topology(tInput)) << tErrorMessage.value();
    }
    {
        // Has mesh name and correct geometry
        auto tInput = input_parser::element_to_node_result_filter{};
        tInput.geometry = cross_reference<input_parser::density_topology>();
        tInput.output_file_name = input_parser::FileName{};
        const auto tErrorMessage = detail::validate_has_mesh_name_or_geometry_is_density_topology(tInput);
        EXPECT_FALSE(detail::validate_has_mesh_name_or_geometry_is_density_topology(tInput)) << tErrorMessage.value();
    }

    // Via registered validation
}

TEST(ElementToNodeResultFilter, Registration)
{
    EXPECT_TRUE(library::is_process_manager_function_registered("element_to_node_result_filter"));
}
}  // namespace plato::process_manager::extension::unittest
