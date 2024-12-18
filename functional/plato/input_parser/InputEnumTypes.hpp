#ifndef PLATO_INPUT_PARSER_INPUTENUMTYPES
#define PLATO_INPUT_PARSER_INPUTENUMTYPES

#include "plato/input_parser/EnumTypeHelpers.hpp"
#include "plato/utilities/EnumTable.hpp"

// clang-format off
DECLARE_ENUM_SYMBOL_TABLE(KernelFilterCenteringTypes,
                         (kElementCentered, "element")
                         (kNodeCentered, "node"))

DECLARE_ENUM_SYMBOL_TABLE(ConstraintTypes,
                         (kEqualTo, "equal_to")
                         (kLessThan, "less_than")
                         (kGreaterThan, "greater_than"))
// clang-format on

#endif
