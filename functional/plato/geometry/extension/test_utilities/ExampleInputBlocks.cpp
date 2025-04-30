#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"

namespace plato::geometry::extension::test_utilities
{
auto create_valid_brick_shape_geometry_input() -> input_parser::brick_shape_geometry
{
    return input_parser::brick_shape_geometry{/*.mesh_name=*/input_parser::FileName{"my_mesh.exo"}};
}

auto create_valid_density_topology_geometry_input() -> input_parser::density_topology
{
    return input_parser::density_topology{/*.mesh_name=*/input_parser::FileName{"test.exo"},
                                          /*.output_name=*/input_parser::FileName{"test_out.exo"},
                                          /*.fixed_blocks=*/{},
                                          /*.filter=*/boost::none,
                                          /*.initial_density_value=*/0.5,
                                          /*.initial_density_field_name=*/boost::none};
}

auto create_valid_level_set_topology_geometry_input() -> input_parser::level_set_topology
{
    return input_parser::level_set_topology{/*.mesh_name = */ input_parser::FileName{"bg.exo"},
                                            /*.output_mesh_name = */ input_parser::FileName{"out.exo"},
                                            /*.include_void_region = */ false,
                                            /*.sphere_pattern_bbox_min_x = */ 0.0,
                                            /*.sphere_pattern_bbox_min_y = */ 0.0,
                                            /*.sphere_pattern_bbox_min_z = */ 0.0,
                                            /*.sphere_pattern_bbox_max_x = */ 1.0,
                                            /*.sphere_pattern_bbox_max_y = */ 1.0,
                                            /*.sphere_pattern_bbox_max_z = */ 1.0,
                                            /*.sphere_pattern_radius = */ 0.25,
                                            /*.sphere_pattern_spacing = */ 100.0,
                                            /*.level_set_lower_bound = */ -1.0,
                                            /*.level_set_upper_bound = */ 1.0,
                                            /*.filter=*/boost::none,
                                            /*.fixed_blocks=*/boost::none,
                                            /*.initial_field_name=*/boost::none};
}

auto create_valid_level_set_topology_geometry_initialize_from_field_input() -> input_parser::level_set_topology
{
    return input_parser::level_set_topology{/*.mesh_name = */
                                            input_parser::FileName{"mesh.exo"},
                                            /*.output_name = */ input_parser::FileName{"level-set-output.exo"},
                                            /*.include_void_region = */ true,
                                            /*.sphere_pattern_bbox_min_x = */ boost::none,
                                            /*.sphere_pattern_bbox_min_y = */ boost::none,
                                            /*.sphere_pattern_bbox_min_z = */ boost::none,
                                            /*.sphere_pattern_bbox_max_x = */ boost::none,
                                            /*.sphere_pattern_bbox_max_y = */ boost::none,
                                            /*.sphere_pattern_bbox_max_z = */ boost::none,
                                            /*.sphere_pattern_radius = */ boost::none,
                                            /*.sphere_pattern_spacing = */ boost::none,
                                            /*.level_set_lower_bound = */ -1.0,
                                            /*.level_set_upper_bound = */ 1.0,
                                            /*.filter=*/boost::none,
                                            /*.fixed_blocks=*/boost::none,
                                            /*.initial_field_name=*/input_parser::IdentifierString{"density"}};
}
}  // namespace plato::geometry::extension::test_utilities
