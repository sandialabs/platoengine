#include <gtest/gtest.h>

#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/filter/extension/HelmholtzFilter.hpp"
#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/filter/extension/KernelFilter.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/process_manager/extension/SensitivityCheck.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"

namespace plato::integration_tests::unittest
{
namespace
{
const auto kValidInputBase = geometry::extension::create_valid_density_topology_geometry_input() |
                             criteria::library::create_valid_example_objective_input() |
                             process_manager::extension::create_valid_example_sensitivity_check_input();
}

template <typename FilterInput>
void check_filter_validation(const FilterInput& aBadFilter, const test_utilities::TestContext& aTestContext)
{
    const auto tInvalidInput = kValidInputBase | aBadFilter;
    EXPECT_TRUE(input_validation::make_validated_input(tInvalidInput).hasError()) << aTestContext;
}

TEST(FilterValidation, ValidIdentityFilter)
{
    const auto tInput = kValidInputBase | filter::extension::create_valid_identity_filter_input();
    EXPECT_TRUE(input_validation::make_validated_input(tInput).hasValue());
}

TEST(FilterValidation, CheckNoFilterRadiusIdentity)
{
    auto tIdentityFilter = filter::extension::create_valid_identity_filter_input();
    tIdentityFilter.filter_radius = 1;  // make invalid
    check_filter_validation(tIdentityFilter, TEST_CONTEXT("Bad identity filter"));
}

TEST(FilterValidation, ValidHelmholtzFilter)
{
    const auto tInput = kValidInputBase | filter::extension::create_valid_helmholtz_filter_input();
    EXPECT_TRUE(input_validation::make_validated_input(tInput).hasValue());
}

TEST(FilterValidation, CheckFilterValuesHelmholtzRadiusBounds)
{
    auto tHelmholtzFilter = filter::extension::create_valid_helmholtz_filter_input();
    tHelmholtzFilter.filter_radius = -1;
    check_filter_validation(tHelmholtzFilter, TEST_CONTEXT("Helmholtz radius bounds"));
}

TEST(FilterValidation, CheckFilterValuesHelmholtzBoundaryStickingPenalty)
{
    auto tHelmholtzFilter = filter::extension::create_valid_helmholtz_filter_input();
    tHelmholtzFilter.boundary_sticking_penalty = -1;
    check_filter_validation(tHelmholtzFilter, TEST_CONTEXT("Helmholtz sticking penalty"));
}

TEST(FilterValidation, ValidKernelFilter)
{
    const auto tInput = kValidInputBase | filter::extension::create_valid_kernel_filter_input();
    EXPECT_TRUE(input_validation::make_validated_input(tInput).hasValue());
}

TEST(FilterValidation, CheckFilterValuesKernelRadiusBounds)
{
    auto tFilter = filter::extension::create_valid_kernel_filter_input();
    tFilter.filter_radius = -0.1;

    check_filter_validation(tFilter, TEST_CONTEXT("Kernel filter radius bounds"));
}

TEST(FilterValidation, CheckFilterValuesKernelCenteringType)
{
    auto tFilter = filter::extension::create_valid_kernel_filter_input();
    tFilter.centering_type = boost::none;
    check_filter_validation(tFilter, TEST_CONTEXT("Kernel filter centering type"));
}

TEST(FilterValidation, CheckFilterValuesHelmholtzRadiusWithMesh)
{
    const auto tMeshFileName = std::filesystem::path{"test.exo"};

    const auto tCommandGenerator = third_party_integration::stk_io::CommandGenerator{
        {2, 2, 2}, {-1, -1, -1}, {1, 1, 1}, third_party_integration::stk_io::CommandElementType::Hex};
    third_party_integration::stk_io::write_mesh(tMeshFileName, tCommandGenerator);

    auto tHelmholtzFilter = filter::extension::create_valid_helmholtz_filter_input();
    tHelmholtzFilter.filter_radius = 0.5;

    check_filter_validation(tHelmholtzFilter, TEST_CONTEXT("Filter radius too small"));

    test_utilities::test_for_existence_and_remove({tMeshFileName}, TEST_CONTEXT("Checking existence of mesh file"));
}

}  // namespace plato::integration_tests::unittest
