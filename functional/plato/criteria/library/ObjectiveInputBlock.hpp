#ifndef PLATO_CRITERIA_OBJECTIVEINPUTBLOCK
#define PLATO_CRITERIA_OBJECTIVEINPUTBLOCK

#include "plato/input_parser/EnumTypeHelpers.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"

// clang-format off
DECLARE_ENUM_AND_ENUM_SYMBOL_TABLE(ObjectiveGoal, 
                                   plato::criteria::library, 
                                  (kMinimize, "minimize")
                                  (kMinimizeNegation, "minimize-negation")
                                  (kMinimizeReciprocal, "minimize-reciprocal"))

PLATO_NAMED_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), objective, plato::components::ComponentType::kObjective,
    (bool, active, "Optional field to de/activate this objective.")
    (plato::input_parser::AppName, app, "Required field specifying the app to use for this objective.") 
    (plato::input_parser::CriterionName, criterion, "Required field specifying the name of the criterion to be evaluated.")
    (unsigned int, number_of_processors, "Optional parameter that will specify the number of processors used to run the app that computes this objective. "
                                         "Requires the 'plato' executable be called with mpirun.")
    (plato::input_parser::FileList, input_files, "Optional comma-separated list of files needed for the app to run.")
    (double, aggregation_weight, "Required weight used to aggregate this objective with any other objectives specified.")
    (plato::criteria::library::ObjectiveGoal, objective_goal, "Optional field specifying whether to minimize an objective, minimize its negation, or minimize its reciprocal. "
                                                              "Default is 'minimize'. Choosing 'minimize-negation' will maximize the objective by minimizing its negation. " 
                                                              "Choosing 'minimize-reciprocal' will maximize the objective by minimizing its reciprocal.")
)
// clang-format off

#endif
