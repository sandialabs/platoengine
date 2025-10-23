#ifndef PLATO_CRITERIA_CONSTRAINTINPUTBLOCK
#define PLATO_CRITERIA_CONSTRAINTINPUTBLOCK

#include "plato/criteria/library/ComponentAndTarget.hpp"
#include "plato/input_parser/EnumTypeHelpers.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"

namespace plato::criteria::library
{
using ConstraintValueList = input_parser::AutoList<input_parser::ComponentAndTarget>;
}

// clang-format off
DECLARE_ENUM_AND_ENUM_SYMBOL_TABLE(ConstraintTypes,
                                   plato::input_parser,
                                  (kEqualTo, "equal_to")
                                  (kLessThan, "less_than")
                                  (kGreaterThan, "greater_than"))

PLATO_NAMED_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), constraint, plato::components::ComponentType::kConstraint,
    (bool, active, "Optional field to de/activate this constraint.")
    (plato::input_parser::AppName, app, "Required field specifying the app to use for this constraint.") 
    (plato::input_parser::CriterionName, criterion, "Required field specifying the name of the criterion to be evaluated.")
    (unsigned int, number_of_processors, "Optional parameter that will specify the number of processors used to run the app that computes this objective. "
                                         "Requires the 'plato' executable be called with mpirun.")
    (plato::input_parser::FileList, input_files, "Optional comma-separated list of files needed for the app to run.")
    (plato::criteria::library::ConstraintValueList, constraint_value_list, "Optional list of component names and target values, in the format: "
        "\"component mass target 42.0\". Only usable by vector constraints to select a subset of vector components.")
    (double, constraint_value, "Required value that the constraint is evaluated against.")
    (bool, is_linear, "Required parameter specifying whether the constraint is linear with respect to the controls.")
    (plato::input_parser::ConstraintTypes, constraint_type, "Required parameter specifying the type of constraint to enforce, 'equal_to', 'less_than', or 'greater_than'.")
)
// clang-format on

#endif
