#include <gtest/gtest.h>

#include <variant>

#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/test_utilities/ValidInputTestFixture.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::process_manager::library::unittest
{
namespace
{
struct ValidatedInputFileFixture : public test_utilities::ValidInputTestFixture
{
};

void test_identity_filter_cross_reference(const ValidatedInput& aInput, const test_utilities::TestContext& aTestContext)
{
    const auto tValidatedGeometry = aInput.geometry().rawInput();
    ASSERT_TRUE(
        std::holds_alternative<core::ValidatedInputTypeWrapper<input_parser::density_topology>>(tValidatedGeometry))
        << aTestContext;
    const auto& tGeometry =
        std::get<core::ValidatedInputTypeWrapper<input_parser::density_topology>>(tValidatedGeometry);
    const auto& tCrossReferencedFilter = tGeometry.rawInput().filter;
    ASSERT_TRUE(tCrossReferencedFilter) << aTestContext;
    ASSERT_TRUE(tCrossReferencedFilter->mInputBlock.holds_expected_type<plato::filter::library::ValidatedFilterInput>())
        << aTestContext;

    const auto tValidatedFilter =
        tCrossReferencedFilter->mInputBlock.get<plato::filter::library::ValidatedFilterInput>().rawInput();
    ASSERT_TRUE(
        std::holds_alternative<core::ValidatedInputTypeWrapper<input_parser::identity_filter>>(tValidatedFilter))
        << aTestContext;
    EXPECT_NO_THROW(
        [[maybe_unused]] const auto tFilter =
            std::get<core::ValidatedInputTypeWrapper<input_parser::identity_filter>>(tValidatedFilter).rawInput())
        << aTestContext;
}
}  // namespace

TEST(ValidatedInput, MakeValidInputWithInvalidInput)
{
    EXPECT_THROW(const auto tValidatedInput = make_validated_input(input_parser::ParsedInput{}), utilities::Exception);
}

TEST_F(ValidatedInputFileFixture, MakeValidInputWithValidInput)
{
    EXPECT_NO_THROW(const auto tValidatedInput = make_validated_input(parsedInput()));
}

TEST(ValidatedInput, MakeValidInputWithGradientCheck)
{
    EXPECT_NO_THROW(const auto tValidatedInput =
                        make_validated_input(test_utilities::create_valid_brick_shape_geometry() |
                                             test_utilities::create_valid_example_objective() |
                                             test_utilities::create_valid_example_gradient_check()));
}

TEST_F(ValidatedInputFileFixture, DensityTopologyCrossReference)
{
    const auto tValidatedInput = make_validated_input(
        test_utilities::create_valid_density_topology_geometry() | test_utilities::create_valid_example_objective() |
        test_utilities::create_valid_example_gradient_check() | test_utilities::create_valid_identity_filter());

    test_identity_filter_cross_reference(tValidatedInput, TEST_CONTEXT("Identity filter cross reference"));
}

TEST_F(ValidatedInputFileFixture, DensityTopologyCrossReferenceWithName)
{
    const auto tValidatedInput = process_manager::library::parse_and_validate(
        R"(
          begin density_topology
            mesh_name test.exo
            output_name test_out.exo
            filter identity_filter
          end
          begin helmholtz_filter
            filter_radius 1
          end
          begin identity_filter
          end
            )" +
        test_utilities::create_valid_example_objective_string() +
        test_utilities::create_valid_example_rol_optimization_string());

    test_identity_filter_cross_reference(tValidatedInput, TEST_CONTEXT("Identity filter cross reference"));
}

}  // namespace plato::process_manager::library::unittest
