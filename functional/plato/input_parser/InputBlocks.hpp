#ifndef PLATO_INPUT_PARSER_INPUTBLOCKS
#define PLATO_INPUT_PARSER_INPUTBLOCKS

#include <string>
#include <vector>

#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"

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
    (plato)(input_parser), rol_optimization,
    (plato::input_parser::FileName, input_file_name)
    (unsigned int, max_iterations)
    (double, step_tolerance)
    (double, gradient_tolerance)
)

PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), gradient_check,
    (plato::input_parser::FileName, output_file_name)
    (unsigned int, number_of_steps)
    (double, initial_direction_magnitude)
    (double, step_size_reduction_factor)
    (unsigned int, random_direction_seed)
)

PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), constraint_check,
    (plato::input_parser::FileName, linearity_check_output_file_name)
    (plato::input_parser::FileName, jacobian_check_output_file_name)
    (plato::input_parser::FileName, jacobian_adjoint_consistency_output_file_name)
    (unsigned int, number_of_steps)
    (double, initial_direction_magnitude)
    (double, step_size_reduction_factor)
    (unsigned int, random_direction_seed)
)

PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), sensitivity_check,
    (plato::input_parser::FileName, output_file_name)
)

PLATO_NAMED_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), objective,
    (bool, active)
    (plato::input_parser::CodeOptions, app) 
    (plato::input_parser::FileName, shared_library_path)
    (unsigned int, number_of_processors)
    (plato::input_parser::FileList, input_files)
    (double, aggregation_weight)
    (plato::input_parser::ObjectiveTypes, objective_type)
)

PLATO_NAMED_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), constraint,
    (bool, active)
    (plato::input_parser::CodeOptions, app) 
    (plato::input_parser::FileName, shared_library_path)
    (unsigned int, number_of_processors)
    (plato::input_parser::FileList, input_files)
    (double, equal_to)
    (bool, is_linear)
)

PLATO_GEOMETRY_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), brick_shape_geometry,
    (plato::input_parser::FileName, mesh_name)
)

PLATO_GEOMETRY_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), density_topology,
    (plato::input_parser::FileName, mesh_name)
    (plato::input_parser::FileName, output_name)
    (plato::input_parser::FilterTypes, filter_type)
    (double, filter_radius)
    (double, boundary_sticking_penalty)
)

/// ParsedInput is the in-memory representation of a parsed input deck.
/// To add new blocks, use PLATO_INPUT_BLOCK_STRUCT or PLATO_NAMED_INPUT_BLOCK_STRUCT
/// macros. The `NAMED` version is for blocks that can have multiple instantiations
/// identified with a name. Those must be added here wrapped in `std::vector`.
BOOST_FUSION_DEFINE_STRUCT(
    (plato)(input_parser), ParsedInput,
    (std::vector<plato::input_parser::objective>, mObjectives)
    (std::vector<plato::input_parser::constraint>, mConstraints)
    (boost::optional<plato::input_parser::brick_shape_geometry>, mBrickShapeGeometry)
    (boost::optional<plato::input_parser::density_topology>, mDensityTopology)
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
