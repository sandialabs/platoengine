#include <gtest/gtest.h>

#include "plato/filter/extension/CommonInputValidation.hpp"
#include "plato/filter/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::filter::extension::unittest
{

TEST(FilterValidation, ValidateNumberOfProcessors)
{
    auto tFilter = test_utilities::create_valid_kernel_filter_input();
    EXPECT_FALSE(detail::validate_number_of_processors(tFilter).has_value());  // valid
    tFilter.number_of_processors = boost::none;
    EXPECT_FALSE(detail::validate_number_of_processors(tFilter).has_value());  // valid - none specified uses 1
}

TEST(FilterValidation, ValidateNumberOfProcessorsFactorOfCommWorld)
{
    auto tFilter = test_utilities::create_valid_kernel_filter_input();
    EXPECT_FALSE(detail::validate_number_of_processors_factor_of_comm_world(tFilter).has_value());  // valid
    tFilter.number_of_processors = 2;
    EXPECT_TRUE(detail::validate_number_of_processors_factor_of_comm_world(tFilter).has_value());  // invalid
}

TEST(FilterValidation, ValidatedIdentityFilter)
{
    auto tIdentityFilter = test_utilities::create_valid_identity_filter_input();

    EXPECT_FALSE(validate_identity_filter(tIdentityFilter).has_value());
    tIdentityFilter.filter_radius = 1;
    EXPECT_TRUE(validate_identity_filter(tIdentityFilter).has_value());
    tIdentityFilter.filter_radius = boost::none;
    EXPECT_FALSE(validate_identity_filter(tIdentityFilter).has_value());
}

TEST(FilterValidation, CheckFilterValuesHelmholtzRadiusBounds)
{
    auto tHelmholtzFilter = test_utilities::create_valid_helmholtz_filter_input();
    EXPECT_FALSE(detail::validate_filter_radius_bounds(tHelmholtzFilter).has_value());  // valid

    tHelmholtzFilter.filter_radius = boost::none;
    EXPECT_TRUE(detail::validate_filter_radius_bounds(tHelmholtzFilter).has_value());
    tHelmholtzFilter.filter_radius = 86.0;
    EXPECT_FALSE(detail::validate_filter_radius_bounds(tHelmholtzFilter).has_value());
    tHelmholtzFilter.filter_radius = -1;
    EXPECT_TRUE(detail::validate_filter_radius_bounds(tHelmholtzFilter).has_value());
}

TEST(FilterValidation, CheckFilterValuesHelmholtzBoundaryStickingPenalty)
{
    auto tHelmholtzFilter = test_utilities::create_valid_helmholtz_filter_input();
    EXPECT_FALSE(validate_helmholtz_filter_boundary_sticking_penalty(tHelmholtzFilter).has_value());  // valid
    tHelmholtzFilter.boundary_sticking_penalty = boost::none;
    EXPECT_FALSE(validate_helmholtz_filter_boundary_sticking_penalty(tHelmholtzFilter).has_value());  // valid, optional
    tHelmholtzFilter.boundary_sticking_penalty = -1;
    EXPECT_TRUE(validate_helmholtz_filter_boundary_sticking_penalty(tHelmholtzFilter).has_value());  // invalid
}

TEST(FilterValidation, CheckFilterValuesKernelRadiusBounds)
{
    auto tFilter = test_utilities::create_valid_kernel_filter_input();
    EXPECT_FALSE(detail::validate_filter_radius_bounds(tFilter).has_value());  // valid

    tFilter.filter_radius = boost::none;
    EXPECT_TRUE(detail::validate_filter_radius_bounds(tFilter).has_value());
    tFilter.filter_radius = 68.0;
    EXPECT_FALSE(detail::validate_filter_radius_bounds(tFilter).has_value());
    tFilter.filter_radius = -0.1;
    EXPECT_TRUE(detail::validate_filter_radius_bounds(tFilter).has_value());
}

TEST(FilterValidation, CheckFilterValuesKernelCenteringType)
{
    auto tFilter = test_utilities::create_valid_kernel_filter_input();
    EXPECT_FALSE(detail::validate_kernel_filter_centering_type(tFilter).has_value());  // valid
    tFilter.centering_type = boost::none;
    EXPECT_TRUE(detail::validate_kernel_filter_centering_type(tFilter).has_value());  // must be defined
    tFilter.centering_type = input_parser::KernelFilterCenteringTypes::kElementCentered;
    EXPECT_FALSE(detail::validate_kernel_filter_centering_type(tFilter).has_value());  // must be defined
}

TEST(FilterValidation, CheckFilterValuesHelmholtzRadiusWithMesh)
{
    const auto tMeshFileName = std::filesystem::path{"test.exo"};

    const auto tCommandGenerator = third_party_integration::stk_io::CommandGenerator{
        {2, 2, 2}, {-1, -1, -1}, {1, 1, 1}, third_party_integration::stk_io::CommandElementType::Hex};
    third_party_integration::stk_io::write_mesh(tMeshFileName, tCommandGenerator);

    auto tHelmholtzFilter = test_utilities::create_valid_helmholtz_filter_input();
    tHelmholtzFilter.filter_radius = 0.5;
    EXPECT_TRUE(detail::validate_filter_radius_with_mesh(tHelmholtzFilter, tMeshFileName)
                    .has_value());  // radius smaller than element edge length of 1
    tHelmholtzFilter.filter_radius = 1.0;
    EXPECT_TRUE(detail::validate_filter_radius_with_mesh(tHelmholtzFilter, tMeshFileName)
                    .has_value());  // radius equal to element edge length of 1
    tHelmholtzFilter.filter_radius = 1.5;
    EXPECT_FALSE(detail::validate_filter_radius_with_mesh(tHelmholtzFilter, tMeshFileName)
                     .has_value());  // radius greater than element edge length of 1

    plato::test_utilities::test_for_existence_and_remove({tMeshFileName},
                                                         TEST_CONTEXT("Checking existence of mesh file"));
}

}  // namespace plato::filter::extension::unittest
