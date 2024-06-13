#include <gtest/gtest.h>

#include "plato/geometry/library/GeometryFactory.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::integration_tests::serial
{
TEST(GeometryFactory, BrickGeometry)
{
    input_parser::ParsedInput tRawInput = test_utilities::create_valid_example_input();
    tRawInput.mDensityTopology = boost::none;
    tRawInput.mBrickShapeGeometry = test_utilities::create_valid_brick_shape_geometry();

    const process_manager::library::ValidatedInput tInput = process_manager::library::make_validated_input(tRawInput);
    const auto tData = geometry::library::make_geometry_data(tInput.geometry());

    constexpr auto tExpectedBrickShapeDimensions = int{6};
    EXPECT_EQ(tData.mInitialGuess.size(), tExpectedBrickShapeDimensions);
    EXPECT_EQ(tData.mBounds.first.size(), tExpectedBrickShapeDimensions);
    EXPECT_EQ(tData.mBounds.second.size(), tExpectedBrickShapeDimensions);
}

}  // namespace plato::integration_tests::serial
