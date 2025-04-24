#ifndef PLATO_GEOMETRY_EXTENSION_TEST_UTILITIES_EXAMPLEINPUTBLOCKS
#define PLATO_GEOMETRY_EXTENSION_TEST_UTILITIES_EXAMPLEINPUTBLOCKS

#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/extension/LevelSetTopology.hpp"

namespace plato::geometry::extension::test_utilities
{
/// @brief Generates a valid brick shape geometry input struct for testing.
[[nodiscard]] auto create_valid_brick_shape_geometry_input() -> input_parser::brick_shape_geometry;

/// @brief Generates a valid example of a density_topology input, useful for testing.
[[nodiscard]] auto create_valid_density_topology_geometry_input() -> input_parser::density_topology;

/// @brief Creates a valid example LevelSetTopology input struct, useful for testing.
[[nodiscard]] auto create_valid_level_set_topology_geometry_input() -> input_parser::level_set_topology;

/// @brief Creates a valid example LevelSetTopology input struct witha field initial guess, useful for testing.
[[nodiscard]] auto create_valid_level_set_topology_geometry_initialize_from_field_input()
    -> input_parser::level_set_topology;
}  // namespace plato::geometry::extension::test_utilities

#endif
