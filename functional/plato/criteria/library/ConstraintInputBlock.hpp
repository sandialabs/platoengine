#ifndef PLATO_CRITERIA_CONSTRAINTINPUTBLOCK
#define PLATO_CRITERIA_CONSTRAINTINPUTBLOCK

#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"

// clang-format off
PLATO_NAMED_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), new_constraint,
    (bool, active, "Optional field to de/activate this constraint.")
    (plato::input_parser::AppName, app, "Required field specifying the app to use for this constraint.") 
    (plato::input_parser::CriterionName, criterion, "Required field specifying the name of the criterion to be evaluated.")
    (unsigned int, number_of_processors, "Optional parameter that will specify the number of processors used to run the app that computes this objective. "
                                         "Requires the 'plato' executable be called with mpirun.")
    (plato::input_parser::FileList, input_files, "Optional comma-separated list of files needed for the app to run.")
    (double, constraint_value, "Required value that the constraint is evaluated against.")
    (bool, is_linear, "Required parameter specifying whether the constraint is linear with respect to the controls.")
    (plato::input_parser::ConstraintTypes, constraint_type, "Required parameter specifying the type of constraint to enforce, 'equal_to', 'less_than', or 'greater_than'.")
)
// clang-format on

namespace plato::criteria::library
{
/// @brief Creates a valid example Constraint struct, useful for testing.
[[nodiscard]] auto create_valid_example_constraint_input() -> input_parser::new_constraint;
}  // namespace plato::criteria::library

#endif
