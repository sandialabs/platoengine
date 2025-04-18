#include <gtest/gtest.h>

#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/filter/extension/HelmholtzFilter.hpp"
#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/filter/extension/KernelFilter.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/integration_tests/utilities/ValidInputTestFixture.hpp"
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

struct FilterValidationTestFixture : public testing::Test
{
    std::filesystem::path mMeshPath =
        kValidInputBase.get<input_parser::density_topology>().front().mesh_name.value().mToken;

    FilterValidationTestFixture()
    {
        const auto tCommandGenerator = third_party_integration::stk_io::CommandGenerator{
            {2, 2, 2}, {-1, -1, -1}, {1, 1, 1}, third_party_integration::stk_io::CommandElementType::Hex};
        third_party_integration::stk_io::write_mesh(mMeshPath, tCommandGenerator);
    }

    ~FilterValidationTestFixture()
    {
        test_utilities::test_for_existence_and_remove({mMeshPath}, TEST_CONTEXT("Checking existence of mesh file"));
    }
};
}  // namespace

template <typename FilterInput>
void check_filter_validation(const FilterInput& aBadFilter, const test_utilities::TestContext& aTestContext)
{
    auto tInvalidInput = kValidInputBase;
    tInvalidInput.get<input_parser::ComponentType::kFilter>().clear();
    tInvalidInput = tInvalidInput | aBadFilter;
    EXPECT_TRUE(input_validation::make_validated_input(tInvalidInput).hasError()) << aTestContext;
}

TEST_F(FilterValidationTestFixture, ValidIdentityFilter)
{
    const auto tInput = kValidInputBase | filter::extension::create_valid_identity_filter_input();
    const auto tValidatedInput = input_validation::make_validated_input(tInput);
    EXPECT_TRUE(tValidatedInput.hasValue()) << tValidatedInput.error();
}

TEST_F(FilterValidationTestFixture, CheckNoFilterRadiusIdentity)
{
    auto tIdentityFilter = filter::extension::create_valid_identity_filter_input();
    tIdentityFilter.filter_radius = 1;  // make invalid
    check_filter_validation(tIdentityFilter, TEST_CONTEXT("Bad identity filter"));
}

TEST_F(FilterValidationTestFixture, ValidHelmholtzFilter)
{
    const auto tInput = kValidInputBase | filter::extension::create_valid_helmholtz_filter_input();
    EXPECT_TRUE(input_validation::make_validated_input(tInput).hasValue());
}

TEST_F(FilterValidationTestFixture, CheckFilterValuesHelmholtzRadiusBounds)
{
    auto tHelmholtzFilter = filter::extension::create_valid_helmholtz_filter_input();
    tHelmholtzFilter.filter_radius = -1;
    check_filter_validation(tHelmholtzFilter, TEST_CONTEXT("Helmholtz radius bounds"));
}

TEST_F(FilterValidationTestFixture, CheckFilterValuesHelmholtzBoundaryStickingPenalty)
{
    auto tHelmholtzFilter = filter::extension::create_valid_helmholtz_filter_input();
    tHelmholtzFilter.boundary_sticking_penalty = -1;
    check_filter_validation(tHelmholtzFilter, TEST_CONTEXT("Helmholtz sticking penalty"));
}

TEST_F(FilterValidationTestFixture, ValidKernelFilter)
{
    const auto tInput = kValidInputBase | filter::extension::create_valid_kernel_filter_input();
    EXPECT_TRUE(input_validation::make_validated_input(tInput).hasValue());
}

TEST_F(FilterValidationTestFixture, CheckFilterValuesKernelRadiusBounds)
{
    auto tFilter = filter::extension::create_valid_kernel_filter_input();
    tFilter.filter_radius = -0.1;

    check_filter_validation(tFilter, TEST_CONTEXT("Kernel filter radius bounds"));
}

TEST_F(FilterValidationTestFixture, CheckFilterValuesKernelCenteringType)
{
    auto tFilter = filter::extension::create_valid_kernel_filter_input();
    tFilter.centering_type = boost::none;
    check_filter_validation(tFilter, TEST_CONTEXT("Kernel filter centering type"));
}

TEST_F(FilterValidationTestFixture, CheckFilterValuesHelmholtzRadiusWithMesh)
{
    auto tHelmholtzFilter = filter::extension::create_valid_helmholtz_filter_input();
    tHelmholtzFilter.filter_radius = 0.5;

    check_filter_validation(tHelmholtzFilter, TEST_CONTEXT("Filter radius too small"));
}

}  // namespace plato::integration_tests::unittest
