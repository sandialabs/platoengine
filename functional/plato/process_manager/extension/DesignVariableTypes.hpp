#ifndef PLATO_PROCESSMANAGER_EXTENSION_DESIGNVARIABLETYPES
#define PLATO_PROCESSMANAGER_EXTENSION_DESIGNVARIABLETYPES

#include "plato/input_parser/EnumTypeHelpers.hpp"

// clang-format off
DECLARE_ENUM_AND_ENUM_SYMBOL_TABLE(DesignVariableTypes,
                                   plato::input_parser,
                                   (kCoordinate, "coordinate")
                                   (kElementDensity, "element_density")
                                   (kNodalDensity, "nodal_density")
                                   )
// clang-format on

#endif
