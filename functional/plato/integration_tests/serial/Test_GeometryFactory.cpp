#include <gtest/gtest.h>

#include "plato/geometry/library/GeometryFactory.hpp"
#include "plato/geometry/library/GeometryRegistrationUtilities.hpp"
#include "plato/main/library/ValidatedInput.hpp"
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
                                  /*.mOptimizationParameters = */ pftu::create_valid_example_optimization_parameters()};

    const main::library::ValidatedInput tInput = main::library::make_validated_input(tRawInput);
    const auto tData = geometry::library::make_geometry_data(tInput.geometry());

    constexpr auto tExpectedBrickShapeDimensions = int{6};
    EXPECT_EQ(tData.mInitialGuess.size(), tExpectedBrickShapeDimensions);
    EXPECT_EQ(tData.mBounds.first.size(), tExpectedBrickShapeDimensions);
    EXPECT_EQ(tData.mBounds.second.size(), tExpectedBrickShapeDimensions);
}

TEST(GeometryFactory, BlockName)
{
    namespace pftu = plato::test_utilities;

    const main::library::ValidatedInput tValidatedInput =
        main::library::make_validated_input(pftu::create_valid_example_input());
    EXPECT_EQ(plato::geometry::library::detail::block_name(tValidatedInput.geometry()), "density_topology");
}
}  // namespace plato::integration_tests::serial
