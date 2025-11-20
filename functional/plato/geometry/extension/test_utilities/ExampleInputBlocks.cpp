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
    return input_parser::level_set_topology{
        /*.mesh_name = */ input_parser::FileName{"bg.exo"},
        /*.output_mesh_name = */ input_parser::FileName{"out.exo"},
        /*.include_void_region = */ false,
        /*.max_edge_length_percentage_for_snapping = */ boost::none,
        /*.sphere_pattern = */
        input_parser::SpherePattern{/*.radius=*/0.25, /*.spacing=*/100.0, /*.min=*/input_parser::Point{0, 0, 0},
                                    /*.max=*/input_parser::Point{1, 1, 1}},
        /*.level_set_bounds = */ input_parser::Bounds{-1.0, 1.0},
        /*.filter=*/boost::none,
        /*.fixed_blocks=*/boost::none,
        /*.initial_field_name=*/boost::none,
        /*.sphere_list=*/boost::none};
}

auto create_valid_level_set_topology_geometry_initialize_from_field_input() -> input_parser::level_set_topology
{
    return input_parser::level_set_topology{/*.mesh_name = */
                                            input_parser::FileName{"mesh.exo"},
                                            /*.output_name = */ input_parser::FileName{"level-set-output.exo"},
                                            /*.include_void_region = */ true,
                                            /*.max_edge_length_percentage_for_snapping = */ boost::none,
                                            /*.sphere_pattern = */ boost::none,
                                            /*.level_set_bounds = */ input_parser::Bounds{-1.0, 1.0},
                                            /*.filter=*/boost::none,
                                            /*.fixed_blocks=*/boost::none,
                                            /*.initial_field_name=*/input_parser::IdentifierString{"density"},
                                            /*.sphere_list=*/boost::none};
}
}  // namespace plato::geometry::extension::test_utilities
