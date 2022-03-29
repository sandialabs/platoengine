/*
 * XMLGeneratorParseMethodInputOptionsUtilities.cpp
 *
 *  Created on: Jan 18, 2022
 */

#include "XMLGeneratorParseMethodInputOptionsUtilities.hpp"

namespace XMLGen
{

/******************************************************************************/
void insert_topology_optimization_input_options(XMLGen::MetaDataTags &aTags)
{
    aTags.insert({ "enforce_bounds", { { {"enforce_bounds"}, ""}, "false" } });
    aTags.insert({ "discretization", { { {"discretization"}, ""}, "density" } });
    aTags.insert({ "initial_density_value", { { {"initial_density_value"}, ""}, "0.5" } });
}
// function insert_topology_optimization_input_options
/******************************************************************************/

/******************************************************************************/
void insert_shape_optimization_input_options(XMLGen::MetaDataTags &aTags)
{
    aTags.insert({ "csm_file", { { {"csm_file"}, ""}, "" } });
    aTags.insert({ "num_shape_design_variables", { { {"num_shape_design_variables"}, ""}, "" } });
}
// function insert_shape_optimization_input_options
/******************************************************************************/

/******************************************************************************/
void insert_general_optimization_input_options(XMLGen::MetaDataTags &aTags)
{
    aTags.insert({ "verbose", { { {"verbose"}, ""}, "false" } });
    aTags.insert({ "output_method", { { {"output_method"}, ""}, "epu" } });
    aTags.insert({ "max_iterations", { { {"max_iterations"}, ""}, "" } });
    aTags.insert({ "output_frequency", { { {"output_frequency"}, ""}, "5" } });
    aTags.insert({ "optimization_type", { { {"optimization_type"}, ""}, "topology" } });
    aTags.insert({ "optimization_algorithm", { { {"optimization_algorithm"}, ""}, "oc" } });
    aTags.insert({ "normalize_in_aggregator", { { {"normalize_in_aggregator"}, ""}, "" } });
    aTags.insert({ "problem_update_frequency", { { {"problem_update_frequency"}, ""}, "5" } });

    // this parameter should be in the objective block. it represents the multiplier applied to the 
    // standard deviation term in robust optimization problems, where the objective is defined as 
    // Mean(f(x)) + k*StdDev(f(x)). Here, k, is the multiplier, i.e. "objective_number_standard_deviations"
    aTags.insert({ "objective_number_standard_deviations", { { {"objective_number_standard_deviations"}, ""}, "" } });
}
// function insert_general_optimization_input_options
/******************************************************************************/

/******************************************************************************/
void insert_rol_input_options(XMLGen::MetaDataTags &aTags)
{
    aTags.insert({ "rol_subproblem_model", { { {"rol_subproblem_model"}, ""}, "" } });
    aTags.insert({ "reset_algorithm_on_update", { { {"reset_algorithm_on_update"}, ""}, "false" } });
    aTags.insert({ "rol_lin_more_cauchy_initial_step_size", { { {"rol_lin_more_cauchy_initial_step_size"}, ""}, "3.0" } });
}
// function insert_rol_input_options
/******************************************************************************/

/******************************************************************************/
void insert_restart_input_options(XMLGen::MetaDataTags &aTags)
{
    aTags.insert({ "restart_iteration", { { {"restart_iteration"}, ""}, "0" } });
    aTags.insert({ "write_restart_file", { { {"write_restart_file"}, ""}, "false" } });
    aTags.insert({ "initial_guess_file_name", { { {"initial_guess_file_name"}, ""}, "" } });
    aTags.insert({ "initial_guess_field_name", { { {"initial_guess_field_name"}, ""}, "" } });
}
// function insert_restart_input_options
/******************************************************************************/

/******************************************************************************/
void insert_derivative_checker_input_options(XMLGen::MetaDataTags &aTags)
{
    aTags.insert({ "check_hessian", { { {"check_hessian"}, ""}, "false" } });
    aTags.insert({ "check_gradient", { { {"check_gradient"}, ""}, "false" } });
    aTags.insert({ "derivative_checker_final_superscript", { { {"derivative_checker_final_superscript"}, ""}, "8" } });
    aTags.insert({ "derivative_checker_initial_superscript", { { {"derivative_checker_initial_superscript"}, ""}, "1" } });
}
// function insert_derivative_checker_input_options
/******************************************************************************/

/******************************************************************************/
void insert_optimality_criteria_input_options(XMLGen::MetaDataTags &aTags)
{
    aTags.insert({ "oc_gradient_tolerance", { { {"oc_gradient_tolerance"}, ""}, "1e-8" } });
    aTags.insert({ "oc_control_stagnation_tolerance", { { {"oc_control_stagnation_tolerance"}, ""}, "1e-2" } });
    aTags.insert({ "oc_objective_stagnation_tolerance", { { {"oc_objective_stagnation_tolerance"}, ""}, "1e-5" } });
}
// function insert_optimality_criteria_input_options
/******************************************************************************/

/******************************************************************************/
void insert_plato_mma_input_options(XMLGen::MetaDataTags &aTags)
{

    aTags.insert({ "mma_move_limit", { { {"mma_move_limit"}, ""}, "0.5" } });
    aTags.insert({ "mma_asymptote_expansion", { { {"mma_asymptote_expansion"}, ""}, "1.2" } });
    aTags.insert({ "mma_asymptote_contraction", { { {"mma_asymptote_contraction"}, ""}, "0.7" } });
    aTags.insert({ "mma_max_sub_problem_iterations", { { {"mma_max_sub_problem_iterations"}, ""}, "50" } });
    aTags.insert({ "mma_sub_problem_initial_penalty", { { {"mma_sub_problem_initial_penalty"}, ""}, "0.0015" } });
    aTags.insert({ "mma_output_subproblem_diagnostics", { { {"mma_output_subproblem_diagnostics"}, ""}, "false" } });
    aTags.insert({ "mma_sub_problem_penalty_multiplier", { { {"mma_sub_problem_penalty_multiplier"}, ""}, "1.025" } });
    aTags.insert({ "mma_use_ipopt_sub_problem_solver", { { {"mma_use_ipopt_sub_problem_solver"}, ""}, "false" } });
    aTags.insert({ "mma_control_stagnation_tolerance", { { {"mma_control_stagnation_tolerance"}, ""}, "1e-6" } });
    aTags.insert({ "mma_objective_stagnation_tolerance", { { {"mma_objective_stagnation_tolerance"}, ""}, "1e-8" } });
    aTags.insert({ "mma_sub_problem_feasibility_tolerance", { { {"mma_sub_problem_feasibility_tolerance"}, ""}, "1e-8" } });
}
// function insert_plato_mma_input_options
/******************************************************************************/

/******************************************************************************/
void insert_amgx_input_options(XMLGen::MetaDataTags &aTags)
{
    aTags.insert({ "amgx_solver_type", { { {"amgx_solver_type"}, ""}, "pbicgstab" } });
    aTags.insert({ "amgx_max_iterations", { { {"amgx_max_iterations"}, ""}, "1000" } });
    aTags.insert({ "amgx_solver_tolerance", { { {"amgx_solver_tolerance"}, ""}, "1e-12" } });
    aTags.insert({ "amgx_print_solver_stats", { { {"amgx_print_solver_stats"}, ""}, "false" } });
}
// function insert_amgx_input_options
/******************************************************************************/

/******************************************************************************/
void insert_fixed_blocks_input_options(XMLGen::MetaDataTags &aTags)
{
    aTags.insert({ "fixed_block_ids", { { {"fixed_block_ids"}, ""}, "" } });
    aTags.insert({ "fixed_sideset_ids", { { {"fixed_sideset_ids"}, ""}, "" } });
    aTags.insert({ "fixed_nodeset_ids", { { {"fixed_nodeset_ids"}, ""}, "" } });
    aTags.insert({ "fixed_block_domain_values", { { {"fixed_block_domain_values"}, ""}, "" } });
    aTags.insert({ "fixed_block_boundary_values", { { {"fixed_block_boundary_values"}, ""}, "" } });
    aTags.insert({ "fixed_block_material_states", { { {"fixed_block_material_states"}, ""}, "" } });
}
// function insert_fixed_blocks_input_options
/******************************************************************************/

/******************************************************************************/
void insert_prune_and_refine_input_options(XMLGen::MetaDataTags &aTags)
{
    aTags.insert({ "prune_mesh", { { {"prune_mesh"}, ""}, "" } });
    aTags.insert({ "number_refines", { { {"number_refines"}, ""}, "" } });
    aTags.insert({ "number_buffer_layers", { { {"number_buffer_layers"}, ""}, "" } });
    aTags.insert({ "prune_and_refine_path", { { {"prune_and_refine_path"}, ""}, "" } });
    aTags.insert({ "number_prune_and_refine_processors", { { {"number_prune_and_refine_processors"}, ""}, "" } });
}
// function insert_prune_and_refine_input_options
/******************************************************************************/

/******************************************************************************/
void insert_general_dakota_input_options(XMLGen::MetaDataTags &aTags)
{
    aTags.insert({ "dakota_workflow", { { {"dakota_workflow"}, ""}, "" } });
    aTags.insert({ "concurrent_evaluations", { { {"concurrent_evaluations"}, ""}, "" } });
}
// function insert_general_dakota_input_options
/******************************************************************************/

/******************************************************************************/
void insert_dakota_multidim_param_study_input_options(XMLGen::MetaDataTags &aTags)
{
    // Dakota Multi-Dimensional Parameter Study (MDPS)
    aTags.insert({ "mdps_partitions", { { {"mdps_partitions"}, ""}, "" } }); // parameter space grid/partition
    aTags.insert({ "mdps_response_functions", { { {"mdps_response_functions"}, ""}, "" } }); // number of response functions analyzed
}
// function insert_dakota_multidim_param_study_input_options
/******************************************************************************/

/******************************************************************************/
void insert_dakota_moga_input_options(XMLGen::MetaDataTags &aTags)
{
    // Dakota Multi-Objective Genetic Algorithm (MOGA)
    aTags.insert({ "sbgo_max_iterations", { { {"sbgo_max_iterations"}, ""}, "10" } });
    aTags.insert({ "moga_population_size", { { {"moga_population_size"}, ""}, "300" } });
    aTags.insert({ "moga_niching_distance", { { {"moga_niching_distance"}, ""}, "0.2" } });
    aTags.insert({ "num_sampling_method_samples", { { {"num_sampling_method_samples"}, ""}, "15" } }); 
    aTags.insert({ "moga_max_function_evaluations", { { {"moga_max_function_evaluations"}, ""}, "20000" } });
    aTags.insert({ "sbgo_surrogate_output_name", { { {"sbgo_surrogate_output_name"}, ""}, "" } });
}
// function insert_dakota_moga_input_options
/******************************************************************************/

/******************************************************************************/
void insert_plato_filter_input_options(XMLGen::MetaDataTags &aTags)
{
    // general filter options
    aTags.insert({ "filter_type", { { {"filter_type"}, ""}, "kernel" } });
    aTags.insert({ "filter_service", { { {"filter_service"}, ""}, "" } });
    aTags.insert({ "filter_in_engine", { { {"filter_in_engine"}, ""}, "true" } });
    aTags.insert({ "filter_radius_scale", { { {"filter_radius_scale"}, ""}, "2.0" } });
    aTags.insert({ "filter_radius_absolute", { { {"filter_radius_absolute"}, ""}, "" } });
    
    // kernel filter specific options
    aTags.insert({ "filter_power", { { {"filter_power"}, ""}, "1" } });
    aTags.insert({ "filter_type_identity_generator_name", { { {"filter_type_identity_generator_name"}, ""}, "identity" } });
    aTags.insert({ "filter_type_kernel_generator_name", { { {"filter_type_kernel_generator_name"}, ""}, "kernel" } });
    aTags.insert({ "filter_type_kernel_then_heaviside_generator_name", { { {"filter_type_kernel_then_heaviside_generator_name"}, ""}, "kernel_then_heaviside" } });
    aTags.insert({ "filter_type_kernel_then_tanh_generator_name", { { {"filter_type_kernel_then_tanh_generator_name"}, ""}, "kernel_then_tanh" } });

    // helmholtz filter specific options
    aTags.insert({ "boundary_sticking_penalty", { { {"boundary_sticking_penalty"}, ""}, "1.0" } });
    aTags.insert({ "symmetry_plane_location_names", { { {"symmetry_plane_location_names"}, ""}, "" } });
}
// function insert_plato_filter_input_options
/******************************************************************************/

/******************************************************************************/
void insert_plato_symmetry_filter_input_options(XMLGen::MetaDataTags &aTags)
{
    aTags.insert({ "symmetry_plane_normal", { { {"symmetry_plane_normal"}, ""}, "" } });
    aTags.insert({ "symmetry_plane_origin", { { {"symmetry_plane_origin"}, ""}, "" } });
    aTags.insert({ "mesh_map_filter_radius", { { {"mesh_map_filter_radius"}, ""}, "" } });
    aTags.insert({ "filter_before_symmetry_enforcement", { { {"filter_before_symmetry_enforcement"}, ""}, "" } });
}
// function insert_plato_symmetry_filter_input_options
/******************************************************************************/

void insert_plato_projection_filter_input_options(XMLGen::MetaDataTags &aTags)
{
    aTags.insert({ "projection_type", { { {"projection_type"}, ""}, "" } });
    aTags.insert({ "filter_heaviside_min", { { {"filter_heaviside_min"}, ""}, "" } });
    aTags.insert({ "filter_heaviside_max", { { {"filter_heaviside_max"}, ""}, "" } });
    aTags.insert({ "filter_heaviside_scale", { { {"filter_heaviside_scale"}, ""}, "" } });
    aTags.insert({ "filter_heaviside_update", { { {"filter_heaviside_update"}, ""}, "" } });
    aTags.insert({ "filter_use_additive_continuation", { { {"filter_use_additive_continuation"}, ""}, "" } });
    aTags.insert({ "filter_projection_start_iteration", { { {"filter_projection_start_iteration"}, ""}, "" } });
    aTags.insert({ "filter_projection_update_interval", { { {"filter_projection_update_interval"}, ""}, "" } });
}

/******************************************************************************/
void insert_plato_levelset_input_options(XMLGen::MetaDataTags &aTags)
{
    aTags.insert({ "levelset_nodesets", { { {"levelset_nodesets"}, ""}, "" } });
    aTags.insert({ "levelset_sphere_radius", { { {"levelset_sphere_radius"}, ""}, "" } });
    aTags.insert({ "create_levelset_spheres", { { {"create_levelset_spheres"}, ""}, "" } });
    aTags.insert({ "levelset_material_box_min", { { {"levelset_material_box_min"}, ""}, "" } });
    aTags.insert({ "levelset_material_box_max", { { {"levelset_material_box_max"}, ""}, "" } });
    aTags.insert({ "levelset_sphere_packing_factor", { { {"levelset_sphere_packing_factor"}, ""}, "" } });
    aTags.insert({ "levelset_initialization_method", { { {"levelset_initialization_method"}, ""}, "" } });
}
// function insert_plato_levelset_input_options
/******************************************************************************/

/******************************************************************************/
void insert_plato_kelley_sachs_trust_region_input_options(XMLGen::MetaDataTags &aTags)
{
    aTags.insert({ "ks_max_radius_scale", { { {"ks_max_radius_scale"}, ""}, "" } });
    aTags.insert({ "ks_initial_radius_scale", { { {"ks_initial_radius_scale"}, ""}, "" } });
    aTags.insert({ "ks_trust_region_ratio_low", { { {"ks_trust_region_ratio_low"}, ""}, "" } });
    aTags.insert({ "ks_trust_region_ratio_mid", { { {"ks_trust_region_ratio_mid"}, ""}, "" } });
    aTags.insert({ "ks_min_trust_region_radius", { { {"ks_min_trust_region_radius"}, ""}, "" } });
    aTags.insert({ "ks_trust_region_ratio_high", { { {"ks_trust_region_ratio_high"}, ""}, "" } });
    aTags.insert({ "ks_disable_post_smoothing", { { {"ks_disable_post_smoothing"}, ""}, "true" } });
    aTags.insert({ "ks_outer_gradient_tolerance", { { {"ks_outer_gradient_tolerance"}, ""}, "" } });
    aTags.insert({ "ks_outer_stagnation_tolerance", { { {"ks_outer_stagnation_tolerance"}, ""}, "" } });
    aTags.insert({ "ks_max_trust_region_iterations", { { {"ks_max_trust_region_iterations"}, ""}, "5" } });
    aTags.insert({ "ks_outer_stationarity_tolerance", { { {"ks_outer_stationarity_tolerance"}, ""}, "" } });
    aTags.insert({ "ks_trust_region_expansion_factor", { { {"ks_trust_region_expansion_factor"}, ""}, "" } });
    aTags.insert({ "ks_trust_region_contraction_factor", { { {"ks_trust_region_contraction_factor"}, ""}, "" } });
    aTags.insert({ "ks_outer_actual_reduction_tolerance", { { {"ks_outer_actual_reduction_tolerance"}, ""}, "" } });
    aTags.insert({ "ks_outer_control_stagnation_tolerance", { { {"ks_outer_control_stagnation_tolerance"}, ""}, "" } });

    aTags.insert({ "hessian_type", { { {"hessian_type"}, ""}, "" } });
    aTags.insert({ "use_mean_norm", { { {"use_mean_norm"}, ""}, "" } });
    aTags.insert({ "al_penalty_parameter", { { {"al_penalty_parameter"}, ""}, "" } });
    aTags.insert({ "feasibility_tolerance", { { {"feasibility_tolerance"}, ""}, "" } });
    aTags.insert({ "limited_memory_storage", { { {"limited_memory_storage"}, ""}, "8" } });
    aTags.insert({ "max_trust_region_radius", { { {"max_trust_region_radius"}, ""}, "" } });
    aTags.insert({ "al_penalty_scale_factor", { { {"al_penalty_scale_factor"}, ""}, "" } });
    aTags.insert({ "al_max_subproblem_iterations", { { {"al_max_subproblem_iterations"}, ""}, "" } });
}
// function insert_plato_kelley_sachs_trust_region_input_options
/******************************************************************************/

}
// namespace XMLGen
