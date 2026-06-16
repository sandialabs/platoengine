#include <gtest/gtest.h>

#include <optional>

#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/filter/extension/kernel_filters/CanonicalKernelFilter.hpp"
#include "plato/filter/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/process_manager/extension/NodalResultFilter.hpp"
#include "plato/process_manager/extension/test_utilities/CrossReferenceUtilities.hpp"
#include "plato/process_manager/extension/test_utilities/NodalResultFilterUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/test_utilities/FileCreatingTestFixture.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::process_manager::extension::unittest
{
namespace
{

class NodalResultFilterValidationFixture : public plato::test_utilities::FileCreatingTestFixture
{
   public:
    NodalResultFilterValidationFixture()
        : FileCreatingTestFixture{
              geometry::extension::test_utilities::create_valid_density_topology_geometry_input().output_name->mToken}
    {
    }
};

using NodalResultSerialFixture = test_utilities::NodalResultFilterRunFixture;

}  // namespace

TEST_F(NodalResultSerialFixture, CreateAndRunSeparateOutputFile)
{
    const auto tOutputMeshPath = mDirectory.directory() / std::filesystem::path{"output.exo"};
    runAndCheckNodalFilterResult(tOutputMeshPath, TEST_CONTEXT("With a new output file"));
    runAndCheckNodalFilterResult(std::nullopt, TEST_CONTEXT("Overwrite existing output file"));
}

TEST(NodalResultFilter, ValidateFilterIsKernelFilter)
{
    {
        // Wrong filter
        auto tInput = input_parser::nodal_result_filter{};
        tInput.filter = test_utilities::cross_reference<input_parser::identity_filter>();
        EXPECT_TRUE(detail::validate_filter_is_kernel_filter(tInput));
    }
    {
        // Correct filter
        auto tInput = input_parser::nodal_result_filter{};
        tInput.filter = test_utilities::cross_reference<input_parser::kernel_filter>();
        const auto tErrorMessage = detail::validate_filter_is_kernel_filter(tInput);
        EXPECT_FALSE(tErrorMessage) << tErrorMessage.value();
    }
}

TEST_F(NodalResultFilterValidationFixture, ValidateFilterIsKernelFilter)
{
    // Via registered validation
    const auto tInput = input_parser::nodal_result_filter{} |
                        geometry::extension::test_utilities::create_valid_density_topology_geometry_input() |
                        criteria::library::test_utilities::create_valid_example_objective_input() |
                        filter::extension::test_utilities::create_valid_identity_filter_input();

    // Wrong filter
    const auto tValidatedInput = input_validation::make_validated_input(tInput);
    EXPECT_TRUE(tValidatedInput.hasError());
}

TEST(NodalResultFilter, ValidateGeometryIsDensityTopology)
{
    {
        // Wrong geometry
        auto tInput = input_parser::nodal_result_filter{};
        tInput.geometry = test_utilities::cross_reference<input_parser::brick_shape_geometry>();
        EXPECT_TRUE(detail::validate_geometry_is_density_topology(tInput));
    }
    {
        // Correct geometry
        auto tInput = input_parser::nodal_result_filter{};
        tInput.geometry = test_utilities::cross_reference<input_parser::density_topology>();
        const auto tErrorMessage = detail::validate_geometry_is_density_topology(tInput);
        EXPECT_FALSE(tErrorMessage) << tErrorMessage.value();
    }
}

TEST_F(NodalResultFilterValidationFixture, ValidateGeometryIsDensityTopology)
{
    // Via registered validation
    const auto tInput = input_parser::nodal_result_filter{} |
                        geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                        criteria::library::test_utilities::create_valid_example_objective_input() |
                        filter::extension::test_utilities::create_valid_kernel_filter_input();

    const auto tValidatedInput = input_validation::make_validated_input(tInput);
    EXPECT_TRUE(tValidatedInput.hasError());
}

TEST(NodalResultFilter, Registration)
{
    EXPECT_TRUE(library::is_process_manager_function_registered("nodal_result_filter"));
}
}  // namespace plato::process_manager::extension::unittest
