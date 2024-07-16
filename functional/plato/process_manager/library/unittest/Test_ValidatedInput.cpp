#include <gtest/gtest.h>

#include <variant>

#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::process_manager::library::unittest
{
namespace
{
void test_helmholtz_filter_cross_reference(const ValidatedInput& aInput, const double aFilterRadius)
{
    const auto tValidatedGeometry = aInput.geometry().rawInput();
    ASSERT_TRUE(
        std::holds_alternative<core::ValidatedInputTypeWrapper<input_parser::density_topology>>(tValidatedGeometry));
    const auto& tGeometry =
        std::get<core::ValidatedInputTypeWrapper<input_parser::density_topology>>(tValidatedGeometry);
    const auto& tCrossReferencedFilter = tGeometry.rawInput().filter;
    ASSERT_TRUE(tCrossReferencedFilter);
    ASSERT_TRUE(
        tCrossReferencedFilter->mInputBlock.holds_expected_type<plato::filter::library::ValidatedFilterInput>());

    const auto tValidatedFilter =
        tCrossReferencedFilter->mInputBlock.get<plato::filter::library::ValidatedFilterInput>().rawInput();
    ASSERT_TRUE(
        std::holds_alternative<core::ValidatedInputTypeWrapper<input_parser::helmholtz_filter>>(tValidatedFilter));
    const auto& tFilter =
        std::get<core::ValidatedInputTypeWrapper<input_parser::helmholtz_filter>>(tValidatedFilter).rawInput();
    ASSERT_TRUE(tFilter.filter_radius);
    EXPECT_EQ(tFilter.filter_radius.value(), aFilterRadius);
}
}  // namespace

TEST(ValidatedInput, MakeValidInputWithInvalidInput)
{
    EXPECT_THROW(const auto tValidatedInput = make_validated_input(input_parser::ParsedInput{}), utilities::Exception);
}

TEST(ValidatedInput, MakeValidInputWithValidInput)
{
    EXPECT_NO_THROW(const auto tValidatedInput = make_validated_input(test_utilities::create_valid_example_input()));
}

TEST(ValidatedInput, MakeValidInputWithGradientCheck)
{
    EXPECT_NO_THROW(const auto tValidatedInput =
                        make_validated_input(test_utilities::create_valid_brick_shape_geometry() |
                                             test_utilities::create_valid_example_objective() |
                                             test_utilities::create_valid_example_gradient_check()));
}

TEST(ValidatedInput, DensityTopologyCrossReference)
{
    const auto tValidatedInput = make_validated_input(
        test_utilities::create_valid_density_topology_geometry() | test_utilities::create_valid_example_objective() |
        test_utilities::create_valid_example_gradient_check() | test_utilities::create_valid_helmholtz_filter());

    constexpr double tFilterRadius = 91.0;
    test_helmholtz_filter_cross_reference(tValidatedInput, tFilterRadius);
}

TEST(ValidatedInput, DensityTopologyCrossReferenceWithName)
{
    constexpr double tFilterRadius = 77.0;

    const auto tValidatedInput = process_manager::library::parse_and_validate(
        R"(
          begin density_topology
            mesh_name my_mesh.exo
            output_name test_out.exo
            filter helmholtz_filter
          end
          begin helmholtz_filter
            filter_radius )" +
        std::to_string(tFilterRadius) + R"(
          end
          begin identity_filter
          end
            )" +
        test_utilities::create_valid_example_objective_string() +
        test_utilities::create_valid_example_rol_optimization_string());

    test_helmholtz_filter_cross_reference(tValidatedInput, tFilterRadius);
}

}  // namespace plato::process_manager::library::unittest
