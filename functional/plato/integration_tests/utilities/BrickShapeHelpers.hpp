#ifndef PLATO_INTEGRATION_TESTS_UTILITIES_BRICKSHAPEHELPERS
#define PLATO_INTEGRATION_TESTS_UTILITIES_BRICKSHAPEHELPERS

#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::input_parser
{
class ParsedInput;
}

namespace plato::integration_tests::utilities
{
/// @brief Returns arbitrary controls for a brick geometry.
[[nodiscard]] auto test_brick_controls() -> linear_algebra::DynamicVector<double>;

/// @brief Computes the volume of a brick from its controls.
[[nodiscard]] auto brick_shape_volume_from_controls(const linear_algebra::DynamicVector<double>& aControls) -> double;

/// @brief Constructs a GeometryFunction using the input @a aInput.
[[nodiscard]] auto brick_shape_geometry_from_input(const input_parser::ParsedInput& aInput)
    -> geometry::library::GeometryFunction;
}  // namespace plato::integration_tests::utilities

#endif
