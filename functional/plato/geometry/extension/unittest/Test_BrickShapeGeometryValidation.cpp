#include <gtest/gtest.h>

#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/utilities/Exception.hpp"
namespace plato::geometry::extension::unittest
{
TEST(BrickShapeGeometryValidation, BrickShapeMeshName)
{
    auto tBrickShapeGeometry = plato::test_utilities::create_valid_brick_shape_geometry();
    EXPECT_FALSE(library::detail::validate_mesh_name(tBrickShapeGeometry).has_value());
    EXPECT_TRUE(library::detail::validate_mesh_name(input_parser::brick_shape_geometry{}).has_value());
}
}  // namespace plato::geometry::extension::unittest