#ifndef PLATO_INPUT_PARSER_INPUTENUMTYPES
#define PLATO_INPUT_PARSER_INPUTENUMTYPES

#include "plato/input_parser/EnumTypeHelpers.hpp"
#include "plato/utilities/EnumTable.hpp"

// clang-format off
DECLARE_ENUM_SYMBOL_TABLE(ObjectiveTypes,
                         (kMinimize, "minimize")
                         (kMaximize, "maximize"))

DECLARE_ENUM_SYMBOL_TABLE(KernelFilterCenteringTypes,
                         (kElementCentered, "element")
                         (kNodeCentered, "node"))
// clang-format on

#endif
