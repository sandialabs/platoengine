#include <gtest/gtest.h>

#include "plato/filter/library/FilterFactory.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/integration_tests/utilities/InputGeneration.hpp"
#include "plato/integration_tests/utilities/ValidInputTestFixture.hpp"

namespace plato::integration_tests::unittest
{
namespace
{
struct FilterFactoryFileFixture : public integration_tests::utilities::ValidInputTestFixture
{
};
}  // namespace

TEST_F(FilterFactoryFileFixture, ValidIdentityFilterFromGeometry)
{
    const auto tValidatedGeometry =
        input_validation::make_validated_input(parsedInput()).value().get<input_parser::ComponentType::kGeometry>();
    const auto tFilterInput = input_validation::validated_cross_reference<input_parser::ComponentType::kFilter>(
        tValidatedGeometry, [](const input_parser::density_topology& aRawInput) { return aRawInput.filter; });

    EXPECT_NO_THROW([[maybe_unused]] const auto tFilter = filter::library::make_filter_function(tFilterInput));
}
}  // namespace plato::integration_tests::unittest
