#include "plato/integration_tests/utilities/BrickShapeHelpers.hpp"

#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::integration_tests::utilities
{
auto test_brick_controls() -> linear_algebra::DynamicVector<double>
{
    constexpr auto tDimensionX = 2.0;
    constexpr auto tDimensionY = 4.0;
    constexpr auto tDimensionZ = 6.0;
    constexpr auto tCenterCoordinate = 0.0;
    return linear_algebra::DynamicVector<double>{tCenterCoordinate, tCenterCoordinate, tCenterCoordinate,
                                                 tDimensionX,       tDimensionY,       tDimensionZ};
}

auto brick_shape_volume_from_controls(const linear_algebra::DynamicVector<double>& aControls) -> double
{
    const auto tDimensionX = aControls[3];
    const auto tDimensionY = aControls[4];
    const auto tDimensionZ = aControls[5];
    return tDimensionX * tDimensionY * tDimensionZ;
}

auto brick_shape_geometry_from_input(const input_parser::ParsedInput& aInput) -> geometry::library::GeometryFunction
{
    const auto tMeshFileName = aInput.get<input_parser::brick_shape_geometry>().front().mesh_name.value().mToken;
    return geometry::extension::make_brick_shape_geometry(geometry::extension::BrickShapeGeometry{tMeshFileName});
}
}  // namespace plato::integration_tests::utilities
