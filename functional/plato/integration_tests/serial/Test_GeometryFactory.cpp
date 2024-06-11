#include <gtest/gtest.h>

#include "plato/geometry/library/GeometryFactory.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::integration_tests::serial
{
TEST(GeometryFactory, BrickGeometry)
{
    namespace pftu = plato::test_utilities;

    const auto tRawInput =
        input_parser::ParsedInput{/*.mObjectives=*/{pftu::create_valid_example_objective()},
                                  /*.mConstraints=*/{pftu::create_valid_example_constraint()},
                                  /*.mBrickShapeGeometry=*/pftu::create_valid_brick_shape_geometry(),
                                  /*.mDensityTopology = */ boost::none,
                                  /*.mLevelsetTopology = */ boost::none,
                                  /*.mROLOptimization = */ pftu::create_valid_example_rol_optimization(),
                                  /*.mGradientCheck = */ boost::none,
                                  /*.mSensitivityCheck = */ boost::none};

    const process_manager::library::ValidatedInput tInput = process_manager::library::make_validated_input(tRawInput);
    const auto tData = geometry::library::make_geometry_data(tInput.geometry());

    constexpr auto tExpectedBrickShapeDimensions = int{6};
    EXPECT_EQ(tData.mInitialGuess.size(), tExpectedBrickShapeDimensions);
    EXPECT_EQ(tData.mBounds.first.size(), tExpectedBrickShapeDimensions);
    EXPECT_EQ(tData.mBounds.second.size(), tExpectedBrickShapeDimensions);
}

}  // namespace plato::integration_tests::serial
