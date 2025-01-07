#ifndef PLATO_INPUT_PARSER_INPUTBLOCKS
#define PLATO_INPUT_PARSER_INPUTBLOCKS

#include <string>
#include <vector>

#include "plato/input_parser/CrossReference.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/HelpDocumentationRegistration.hpp"
#include "plato/input_parser/IdentifierString.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"

namespace plato::input_parser
{
using FilterCrossReference = plato::input_parser::CrossReference<plato::input_parser::IsFilterInput>;
}

/// @file Input block declarations.
///  Each PLATO_INPUT_BLOCK_STRUCT represents a parsable struct of key-value pairs.
///  For example, the `objective` block will be parsed as
/// @code
/// begin objective
///   active true
///   app sd_mass_app
///   ...
/// end objective
/// @endcode
/// and the parsed object has fields corresponding to the input keys.
///
/// Each declaration starts with the namespace the struct is declared in.
/// The second argument is the struct name, and the third argument is a
/// sequence of type/name pairs for each struct field. Any new input blocks
/// must also be added to the main ParsedInput struct.
///
/// Importantly, each field type is wrapped in boost::optional, which is used
/// to indicate if the field was actually present and parsed in the input deck.
///
/// @note `std::string` cannot be used as an argument directly, instead use
/// the helper types FileName and FileList.
/// @note Enumerations may be used, but must be declared with DECLARE_ENUM_SYMBOL_TABLE and
///  defined with DEFINE_ENUM_SYMBOL_TABLE.
// clang-format off
PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), snopt_optimization,
    (plato::input_parser::FileName, input_file_name, "Optional filename of the SNOpt input file to set optimization parameters not otherwise available here.")
    (unsigned int, max_iterations, "Optional command to override the maximum number of outer iterations given in an input file.")
    (unsigned int, time_limit_in_minutes, "Optional command to override the time limit in minutes for this optimization as an additional stopping criteria for the optimization.")
)

PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), rol_optimization,
    (plato::input_parser::FileName, input_file_name, "Optional filename of the XML file specifying the ROL input parameters.")
    (plato::input_parser::FileName, export_settings_file_name, "Optional filename to trigger an exporting of all the set parameters of ROL.")
    (unsigned int, max_iterations, "Optional command to override the maximum number of outer iterations given in an input file.")
    (double, step_tolerance, "Optional command to override the step tolerance given in an input file. The step tolerance is the norm of the change in all of the controls from one step to the next. This number should scale with the problem size.")
    (double, gradient_tolerance, "Optional command to override the gradient tolerance given in an input file. The gradient tolerance is the norm of the gradient which at an optimal solution should be zero. This number should scale with the problem size.")
    (double, initial_search_radius, "Optional command to override the starting search radius for the controls. This number should increase with the problem size.")
    (bool, verbose_output, "Optional command to increase the verbosity of the ROL algorithms to their maximum.")
    (bool, approximate_hessian, "Optional command to override the default zero-Hessian behavior. The Hessian will be approximated with a secant method.")
)

PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), gradient_check,
    (plato::input_parser::FileName, output_file_name, "Required filename to use to report the results of the gradient check.")
    (unsigned int, number_of_steps, "Required field specifying the number of approximations to evaluate using a finite difference.")
    (double, initial_direction_magnitude, "Required field specifying the magnitude of the perturbation of the design controls.")
    (double, step_size_reduction_factor, "Required field specifying how much the perturbation will be reduced for each step, e.g., 0.1 for log10 step sizes.")
    (unsigned int, random_direction_seed, "Required field specifying the seed that is used to generate the random perturbation of the controls.")
)

PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), constraint_check,
    (plato::input_parser::FileName, linearity_check_output_file_name, "Required filename to use to report the results of the linearity check.")
    (plato::input_parser::FileName, jacobian_check_output_file_name, "Required filename to use to report the results of the Jacobian check.")
    (plato::input_parser::FileName, jacobian_adjoint_consistency_output_file_name, "Required filename to use to report the results of the Jacobian and adjoint Jacobian consistency check.")
    (unsigned int, number_of_steps, "Required field specifying the number of approximations to evaluate using a finite difference.")
    (double, initial_direction_magnitude, "Required field specifying the magnitude of the perturbation of the design controls.")
    (double, step_size_reduction_factor, "Required field specifying how much the perturbation will be reduced for each step, e.g., 0.1 for log10 step sizes.")
    (unsigned int, random_direction_seed, "Required field specifying the seed that is used to generate the random perturbation of the controls.")
)

PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), sensitivity_check,
    (plato::input_parser::FileName, output_file_name, "Required filename to use to report the results of the sensitivity check.")
)

PLATO_NAMED_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), objective,
    (bool, active, "Optional field to de/activate this objective.")
    (plato::input_parser::AppName, app, "Required field specifying the app to use for this objective.") 
    (plato::input_parser::CriterionName, criterion, "Required field specifying the name of the criterion to be evaluated.")
    (unsigned int, number_of_processors, "Optional parameter that will specify the number of processors used to run the app that computes this objective. Requires the 'plato' executable be called with mpirun.")
    (plato::input_parser::FileList, input_files, "Optional comma-separated list of files needed for the app to run.")
    (double, aggregation_weight, "Required weight used to aggregate this objective with any other objectives specified.")
)

PLATO_NAMED_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), constraint,
    (bool, active, "Optional field to de/activate this constraint.")
    (plato::input_parser::AppName, app, "Required field specifying the app to use for this constraint.") 
    (plato::input_parser::CriterionName, criterion, "Required field specifying the name of the criterion to be evaluated.")
    (unsigned int, number_of_processors, "Optional parameter that will specify the number of processors used to run the app that computes this objective. Requires the 'plato' executable be called with mpirun.")
    (plato::input_parser::FileList, input_files, "Optional comma-separated list of files needed for the app to run.")
    (double, constraint_value, "Required value that the constraint is evaluated against.")
    (bool, is_linear, "Required parameter specifying whether the constraint is linear with respect to the controls.")
    (plato::input_parser::ConstraintTypes, constraint_type, "Required parameter specifying the type of constraint to enforce, 'equal_to', 'less_than', or 'greater_than'.")
)

PLATO_GEOMETRY_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), brick_shape_geometry,
    (plato::input_parser::FileName, mesh_name, "Required field specifying the exodus mesh name to write the brick mesh output to.")
)

PLATO_GEOMETRY_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), density_topology,
    (plato::input_parser::FileName, mesh_name, "Required field specifying the file name of the exodus mesh to read and generate controls from.")
    (plato::input_parser::FileName, output_name, "Required field specifying the exodus output file name to use when writing results.")
    (plato::input_parser::FixedBlockList, fixed_blocks, "Optional comma separated list of block names that should be fixed in the mesh and not be part of the optimization.")
    (plato::input_parser::FilterCrossReference, filter, "Required name of the filter block to apply to the controls.")
    (double, initial_density_value, "Method to specify a uniform initial density value to give to the controls. Omit if 'initial_density_field_name' is specified.")
    (plato::input_parser::IdentifierString, initial_density_field_name, "Method to read the controls from the specified field name within the 'mesh_name' exodus mesh. Omit if 'initial_density_value' is specified.")
)

PLATO_GEOMETRY_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), level_set_topology,
    (plato::input_parser::FileName, background_mesh_name, "Required field specifying the file name of the exodus mesh to read and generate controls from.")
    (plato::input_parser::FileName, output_mesh_name, "Required field specifying the exodus output file name to use when writing results.")
    (bool, include_void_region, "Required field specifying whether to include the elements of the void region when writing the cut mesh.")
    (double, sphere_pattern_bbox_min_x, "Required field specifying the starting x-coordinate of the sphere pattern's bounding box.")
    (double, sphere_pattern_bbox_min_y, "Required field specifying the starting y-coordinate of the sphere pattern's bounding box.")
    (double, sphere_pattern_bbox_min_z, "Required field specifying the starting z-coordinate of the sphere pattern's bounding box.")
    (double, sphere_pattern_bbox_max_x, "Required field specifying the ending x-coordinate of the sphere pattern's bounding box.")
    (double, sphere_pattern_bbox_max_y, "Required field specifying the ending y-coordinate of the sphere pattern's bounding box.")
    (double, sphere_pattern_bbox_max_z, "Required field specifying the ending z-coordinate of the sphere pattern's bounding box.")
    (double, sphere_pattern_radius, "Required field specifying the radius of the spheres to be inserted in the bounding box. This and the spacing will determine the total number of spheres added inside the bounding box.")
    (double, sphere_pattern_spacing, "Required field specifying the gap between adjacent spheres in the unform pattern.")
    (double, level_set_lower_bound, "Required field specifying the value of the control that sets the lower bound of he level set cut.")
    (double, level_set_upper_bound, "Required field specifying the value of the control that sets the upper bound of he level set cut.")
    (plato::input_parser::FilterCrossReference, filter, "Required name of the filter block to apply to the controls.")
    (plato::input_parser::FixedBlockList, fixed_blocks, "Optional list of blocks in the mesh that will have level-set fields assigned to the level_set_upper_bound value.")
)

PLATO_FILTER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), identity_filter,
    (double, filter_radius, "Do not use - Identity filter cannot have a filter radius.")
)

PLATO_FILTER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), helmholtz_filter,
    (double, filter_radius, "Required field specifying the size of the filter radius.")
    (bool, use_relative_radius, "Optional field that can convert the filter radius specified into a radius relative to the average element size.")
    (double, boundary_sticking_penalty, "Optional field specifying the penalization of material that sticks to boundaries of the design domain as an artifact of filtering. Unit bounded: a value of 1 applies maximum penalization.")
)

PLATO_FILTER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), kernel_filter,
    (double, filter_radius, "Required field specifying the size of the filter radius.")
    (plato::input_parser::KernelFilterCenteringTypes, centering_type, "Required field specifying whether the results should be 'node' or 'element' centered. Platoanalyze requires node-centered and SD requires element-centered.")
    (bool, use_relative_radius, "Optional field that can convert the filter radius specified into a radius relative to the average element size.")
    (unsigned int, number_of_processors, "Optional parameter that will specify the number of processors used to run the filter in parallel. Requires the 'plato' executable be called with mpirun. Limited to the maximum processors specified in the mpirun call. Over-parallelization can cause a significant degredation of the filter.")
)

/// ParsedInput is the in-memory representation of a parsed input deck.
/// To add new blocks, use PLATO_INPUT_BLOCK_STRUCT or PLATO_NAMED_INPUT_BLOCK_STRUCT
/// macros. The `NAMED` version is for blocks that can have multiple instantiations
/// identified with a name. Those must be added here wrapped in `std::vector`.
///
/// @note This macro is what defines the variants for different input block types
/// (e.g. process_managers, geometry representations, filters, etc.). 
/// If you get:
/// "Requested process manager variant missing from available variant types."
/// you most likely forgot to add the new input block to this list.
BOOST_FUSION_DEFINE_STRUCT(
    (plato)(input_parser), ParsedInput,
    (std::vector<plato::input_parser::objective>, mObjectives)
    (std::vector<plato::input_parser::constraint>, mConstraints)
    (boost::optional<plato::input_parser::brick_shape_geometry>, mBrickShapeGeometry)
    (boost::optional<plato::input_parser::density_topology>, mDensityTopology)
    (boost::optional<plato::input_parser::level_set_topology>, mLevelSetTopology)
    (boost::optional<plato::input_parser::identity_filter>, mIdentityFilter)
    (boost::optional<plato::input_parser::helmholtz_filter>, mHelmholtzFilter)
    (boost::optional<plato::input_parser::kernel_filter>, mKernelFilter)
    (boost::optional<plato::input_parser::snopt_optimization>, mSNOPTOptimization)
    (boost::optional<plato::input_parser::rol_optimization>, mROLOptimization)
    (boost::optional<plato::input_parser::gradient_check>, mGradientCheck)
    (boost::optional<plato::input_parser::constraint_check>, mConstraintCheck)
    (boost::optional<plato::input_parser::sensitivity_check>, mSensitivityCheck)
)

// clang-format on
namespace plato::input_parser
{
template <typename BlockStruct>
std::string block_name()
{
    return InputTypeName<BlockStruct>::name;
}
}  // namespace plato::input_parser

#endif
