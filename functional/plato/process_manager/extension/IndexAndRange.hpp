#ifndef PLATO_PROCESSMANAGER_EXTENSION_INDEXANDRANGE
#define PLATO_PROCESSMANAGER_EXTENSION_INDEXANDRANGE

#include "plato/input_parser/Range.hpp"
#include "plato/input_parser/SequenceSubtype.hpp"

///@brief Input struct with parser to read in an index followed by a range definition
// clang-format off
PLATO_INPUT_SEQUENCE_SUBTYPE(
(plato)(input_parser),IndexAndRange,
(int, index)
(plato::input_parser::Range, range)
)
// clang-format on

#endif
