#ifndef PLATO_INPUTBLOCKS_HPP
#define PLATO_INPUTBLOCKS_HPP

#include "Plato_InputBlockStruct.hpp"
#include "Plato_FileList.hpp"
#include "Plato_InputEnumTypes.hpp"

#include <string>
#include <vector>

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
/// must also be added to the main PlatoInput struct.
///
/// Importantly, each field type is wrapped in boost::optional, which is used
/// to indicate if the field was actually present and parsed in the input deck.
///
/// @note `std::string` cannot be used as an argument directly, instead use
/// the helper types FileName and FileList.
/// @note Enumerations may be used, but must be declared with DECLARE_ENUM_SYMBOL_TABLE and
///  defined with DEFINE_ENUM_SYMBOL_TABLE.

PLATO_INPUT_BLOCK_STRUCT(
    (Plato), optimization_parameters,
    (Plato::FileName, input_file_name)
    (unsigned int, max_iterations)
    (double, step_tolerance)
    (double, gradient_tolerance)
)

PLATO_NAMED_INPUT_BLOCK_STRUCT(
    (Plato), objective,
    (bool, active)
    (Plato::CodeOptions, app) 
    (unsigned int, number_of_processors)
    (Plato::FileList, input_files)
    (double, aggregation_weight)
    (Plato::ObjectiveTypes, objective_type)
)

PLATO_NAMED_INPUT_BLOCK_STRUCT(
    (Plato), constraint,
    (bool, active)
    (Plato::CodeOptions, app) 
    (unsigned int, number_of_processors)
    (Plato::FileList, input_files)
    (double, equal_to)
    (double, less_than)
    (double, greater_than)
    (bool, is_linear)
)

/// PlatoInput is the in-memory representation of a parsed input deck.
/// To add new blocks, use PLATO_INPUT_BLOCK_STRUCT or PLATO_NAMED_INPUT_BLOCK_STRUCT
/// macros. The `NAMED` version is for blocks that can have multiple instantiations
/// identified with a name. Those must be added here wrapped in `std::vector`.
BOOST_FUSION_DEFINE_STRUCT(
    (Plato), PlatoInput,
    (std::vector<Plato::objective>, mObjectives)
    (std::vector<Plato::constraint>, mConstraints)
    (Plato::optimization_parameters, mOptimizationParameters)
)

#endif
