#include <gtest/gtest.h>

#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/filter/extension/HelmholtzFilter.hpp"
#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/filter/extension/KernelFilter.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/integration_tests/utilities/InputGeneration.hpp"
#include "plato/process_manager/extension/ROLOptimization.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::integration_tests::unittest
{
namespace
{
void test_cross_linked_filter(const input_parser::CrossLinkedInput& aCrossLinkedInput,
                              const test_utilities::TestContext& aTestContext)
{
    ASSERT_FALSE(aCrossLinkedInput.rawInput().get<input_parser::new_density_topology>().empty());
    const auto tDensityTopologyInput = aCrossLinkedInput.rawInput().get<input_parser::new_density_topology>().front();
    ASSERT_TRUE(tDensityTopologyInput.filter) << aTestContext;
    const auto& tFilterCrossReference = tDensityTopologyInput.filter->mInputBlock;
    EXPECT_TRUE(tFilterCrossReference.hasValue()) << aTestContext;
    EXPECT_TRUE(tFilterCrossReference.template holdsExpectedType<input_parser::helmholtz_filter>()) << aTestContext;
}
}  // namespace

TEST(GeometryValidation, ValidateEmptyFilterAndDensityTopologyInput)
{
    // This configuration was found to throw an exception for an unchecked optional access
    auto tGeometryInput = geometry::extension::create_valid_density_topology_geometry_input();
    tGeometryInput.mesh_name = boost::none;
    tGeometryInput.output_name = boost::none;
    auto tFilterInput = filter::extension::create_valid_kernel_filter_input();
    tFilterInput.filter_radius = boost::none;
    tFilterInput.centering_type = boost::none;

    auto tInput = tGeometryInput | tFilterInput | criteria::library::create_valid_example_objective_input() |
                  process_manager::extension::create_valid_example_rol_optimization_input();

    EXPECT_TRUE(input_validation::make_validated_input(tInput).hasError());
}

TEST(GeometryValidation, ValidateNoCrossLinkedFilter)
{
    // This configuration was found to throw an unchecked optional access exception.
    // The expected filter input block is missing, which should be caught in validation.
    const auto tInput = geometry::extension::create_valid_density_topology_geometry_input() |
                        criteria::library::create_valid_example_objective_input() |
                        process_manager::extension::create_valid_example_rol_optimization_input();

    EXPECT_TRUE(input_validation::make_validated_input(tInput).hasError());
}

TEST(GeometryValidation, LinksDensityTopologyToOnlyFilter)
{
    const auto tInput = geometry::extension::create_valid_density_topology_geometry_input() |
                        filter::extension::create_valid_helmholtz_filter_input();
    ASSERT_FALSE(tInput.get<input_parser::new_density_topology>().front().filter);

    const auto tCrossLinkedInput = input_parser::make_cross_linked_input(tInput);
    ASSERT_TRUE(tCrossLinkedInput.hasValue());
    test_cross_linked_filter(tCrossLinkedInput.value(), TEST_CONTEXT("Only one filter available"));
}

TEST(GeometryValidation, LinksDensityTopologyToSpecifiedFilter)
{
    auto tDensityTopologyInput = geometry::extension::create_valid_density_topology_geometry_input();
    tDensityTopologyInput.filter =
        input_parser::NewCrossReference<input_parser::ComponentType::kFilter>{"helmholtz_filter", {}};

    const auto tInput = tDensityTopologyInput | filter::extension::create_valid_helmholtz_filter_input() |
                        filter::extension::create_valid_identity_filter_input();

    const auto tCrossLinkedInput = input_parser::make_cross_linked_input(tInput);
    ASSERT_TRUE(tCrossLinkedInput.hasValue()) << tCrossLinkedInput.error();
    test_cross_linked_filter(tCrossLinkedInput.value(), TEST_CONTEXT("Filter specified when multiple are available"));
}
}  // namespace plato::integration_tests::unittest
