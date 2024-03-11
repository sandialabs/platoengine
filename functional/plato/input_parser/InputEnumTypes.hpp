#ifndef PLATO_INPUT_PARSER_INPUTENUMTYPES
#define PLATO_INPUT_PARSER_INPUTENUMTYPES

#include "plato/input_parser/EnumTypeHelpers.hpp"
#include "plato/utilities/EnumTable.hpp"

// clang-format off
DECLARE_ENUM_SYMBOL_TABLE(CodeOptions,
                         (kSierraMassApp, "sierra_mass_app")
                         (kNodalSum, "nodal_sum")
                         (kCustomApp, "custom_app"))

DECLARE_ENUM_SYMBOL_TABLE(ObjectiveTypes,
                         (kMinimize, "minimize")
                         (kMaximize, "maximize"))

DECLARE_ENUM_SYMBOL_TABLE(FilterTypes,
                         (kIdentity, "identity")
                         (kHelmholtz, "helmholtz")
                         (kKernel, "kernel"))
// clang-format on

#endif
