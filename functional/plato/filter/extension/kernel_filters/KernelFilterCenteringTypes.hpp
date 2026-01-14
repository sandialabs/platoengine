#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_KERNELFILTERCENTERINGTYPES
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_KERNELFILTERCENTERINGTYPES

#include "plato/input_parser/EnumTypeHelpers.hpp"

DECLARE_ENUM_AND_ENUM_SYMBOL_TABLE(KernelFilterCenteringTypes,
                                   plato::input_parser,
                                   (kElementCentered, "element")(kNodeCentered, "node"))

#endif
