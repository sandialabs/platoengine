#include <gtest/gtest.h>

#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/library/GeometryFactory.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/integration_tests/utilities/InputGeneration.hpp"

namespace plato::integration_tests::serial
{
TEST(GeometryFactory, BrickGeometry)
{
    auto tRawInput = integration_tests::utilities::create_valid_example_input();
    tRawInput.get<input_parser::ComponentType::kGeometry>().clear();
    tRawInput = std::move(tRawInput) | geometry::extension::test_utilities::create_valid_brick_shape_geometry_input();

    const auto tInput = input_validation::make_validated_input(tRawInput).value();
    const auto tData = geometry::library::make_geometry_data(tInput.get<input_parser::ComponentType::kGeometry>());

    constexpr auto tExpectedBrickShapeDimensions = int{6};
    EXPECT_EQ(tData.mInitialGuess.size(), tExpectedBrickShapeDimensions);
    EXPECT_EQ(tData.mBounds.first.size(), tExpectedBrickShapeDimensions);
    EXPECT_EQ(tData.mBounds.second.size(), tExpectedBrickShapeDimensions);
}

}  // namespace plato::integration_tests::serial
