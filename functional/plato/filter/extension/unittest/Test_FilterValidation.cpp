#include <gtest/gtest.h>

#include "plato/core/ValidationRegistration.hpp"
#include "plato/filter/extension/CommonInputValidation.hpp"
#include "plato/filter/extension/HelmholtzFilter.hpp"
#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/filter/extension/KernelFilter.hpp"
#include "plato/filter/library/FilterValidation.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::filter::extension::unittest
{

void check_helmholtz_validation_with_variants(const input_parser::helmholtz_filter& aBadFilter)
{
    input_parser::ParsedInput tInput =
        input_parser::ParsedInput{} | plato::test_utilities::create_valid_helmholtz_filter();
    std::vector<std::string> tErrorMessages;
    tErrorMessages = filter::library::validate_filter(tInput, std::move(tErrorMessages));
    EXPECT_EQ(tErrorMessages.size(), 0u);
    tInput.mHelmholtzFilter = aBadFilter;
    tErrorMessages = filter::library::validate_filter(tInput, std::vector<std::string>{});
    EXPECT_EQ(tErrorMessages.size(), 1u);
}

void check_kernel_validation_with_variants(const input_parser::kernel_filter& aBadFilter)
{
    input_parser::ParsedInput tInput =
        input_parser::ParsedInput{} | plato::test_utilities::create_valid_kernel_filter();
    std::vector<std::string> tErrorMessages;
    tErrorMessages = filter::library::validate_filter(tInput, std::move(tErrorMessages));
    EXPECT_EQ(tErrorMessages.size(), 0u);
    tInput.mKernelFilter = aBadFilter;
    tErrorMessages = filter::library::validate_filter(tInput, std::vector<std::string>{});
    EXPECT_EQ(tErrorMessages.size(), 1u);
}

TEST(FilterValidation, CheckNoFilterRadiusIdentity)
{
    auto tIdentityFilter = plato::test_utilities::create_valid_identity_filter();

    EXPECT_FALSE(validate_identity_filter(tIdentityFilter).has_value());
    tIdentityFilter.filter_radius = 1;
    EXPECT_TRUE(validate_identity_filter(tIdentityFilter).has_value());
    tIdentityFilter.filter_radius = boost::none;
    EXPECT_FALSE(validate_identity_filter(tIdentityFilter).has_value());

    // Check through validation with all variants
    input_parser::ParsedInput tInput =
        input_parser::ParsedInput{} | plato::test_utilities::create_valid_identity_filter();
    std::vector<std::string> tErrorMessages;
    tErrorMessages = filter::library::validate_filter(tInput, std::move(tErrorMessages));
    EXPECT_EQ(tErrorMessages.size(), 0u);
    tIdentityFilter.filter_radius = 1;  // make invalid
    tInput.mIdentityFilter = tIdentityFilter;
    tErrorMessages = filter::library::validate_filter(tInput, std::vector<std::string>{});
    EXPECT_EQ(tErrorMessages.size(), 1u);
}

TEST(FilterValidation, CheckFilterValuesHelmholtzRadius)
{
    auto tHelmholtzFilter = plato::test_utilities::create_valid_helmholtz_filter();
    EXPECT_FALSE(detail::validate_filter_radius(tHelmholtzFilter).has_value());  // valid

    tHelmholtzFilter.filter_radius = boost::none;
    EXPECT_TRUE(detail::validate_filter_radius(tHelmholtzFilter).has_value());
    tHelmholtzFilter.filter_radius = 86.0;
    EXPECT_FALSE(detail::validate_filter_radius(tHelmholtzFilter).has_value());
    tHelmholtzFilter.filter_radius = -1;
    EXPECT_TRUE(detail::validate_filter_radius(tHelmholtzFilter).has_value());

    check_helmholtz_validation_with_variants(tHelmholtzFilter);
}

TEST(FilterValidation, CheckFilterValuesHelmholtzBoundaryStickingPenalty)
{
    auto tHelmholtzFilter = plato::test_utilities::create_valid_helmholtz_filter();
    EXPECT_FALSE(validate_helmholtz_filter_boundary_sticking_penalty(tHelmholtzFilter).has_value());  // valid
    tHelmholtzFilter.boundary_sticking_penalty = boost::none;
    EXPECT_FALSE(validate_helmholtz_filter_boundary_sticking_penalty(tHelmholtzFilter).has_value());  // valid, optional
    tHelmholtzFilter.boundary_sticking_penalty = -1;
    EXPECT_TRUE(validate_helmholtz_filter_boundary_sticking_penalty(tHelmholtzFilter).has_value());  // invalid

    check_helmholtz_validation_with_variants(tHelmholtzFilter);
}

TEST(FilterValidation, CheckFilterValuesKernelRadius)
{
    auto tFilter = plato::test_utilities::create_valid_kernel_filter();
    EXPECT_FALSE(detail::validate_filter_radius(tFilter).has_value());  // valid

    tFilter.filter_radius = boost::none;
    EXPECT_TRUE(detail::validate_filter_radius(tFilter).has_value());
    tFilter.filter_radius = 68.0;
    EXPECT_FALSE(detail::validate_filter_radius(tFilter).has_value());
    tFilter.filter_radius = -0.1;
    EXPECT_TRUE(detail::validate_filter_radius(tFilter).has_value());

    check_kernel_validation_with_variants(tFilter);
}

TEST(FilterValidation, CheckFilterValuesKernelCenteringType)
{
    auto tFilter = plato::test_utilities::create_valid_kernel_filter();
    EXPECT_FALSE(detail::validate_kernel_filter_centering_type(tFilter).has_value());  // valid
    tFilter.centering_type = boost::none;
    EXPECT_TRUE(detail::validate_kernel_filter_centering_type(tFilter).has_value());  // must be defined
    tFilter.centering_type = input_parser::KernelFilterCenteringTypes::kElementCentered;
    EXPECT_FALSE(detail::validate_kernel_filter_centering_type(tFilter).has_value());  // must be defined

    tFilter.centering_type = boost::none;
    check_kernel_validation_with_variants(tFilter);
}

}  // namespace plato::filter::extension::unittest
