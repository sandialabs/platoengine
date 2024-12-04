#include <gtest/gtest.h>

#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::integration_tests::unittest
{
TEST(DensityTopology, ValidateEmptyFilterAndDensityTopologyInput)
{
    auto tInput = test_utilities::create_valid_density_topology_geometry() |
                  test_utilities::create_valid_kernel_filter() | test_utilities::create_valid_example_objective() |
                  test_utilities::create_valid_example_rol_optimization();
    // Clear all inputs
    // This configuration was found to throw an exception for an unchecked optional access
    tInput.mDensityTopology->mesh_name = boost::none;
    tInput.mDensityTopology->output_name = boost::none;
    tInput.mKernelFilter->filter_radius = boost::none;
    tInput.mKernelFilter->centering_type = boost::none;

    EXPECT_THROW([[maybe_unused]] const auto tValidatedInput = process_manager::library::make_validated_input(tInput),
                 plato::utilities::Exception);
}
}  // namespace plato::integration_tests::unittest
