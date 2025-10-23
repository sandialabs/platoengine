#ifndef PLATO_CRITERIA_LIBRARY_COMPONENTANDTARGET
#define PLATO_CRITERIA_LIBRARY_COMPONENTANDTARGET

#include <string>

#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/input_parser/SequenceSubtype.hpp"

// clang-format off
///@brief Input struct with parser to read in an index followed by a range definition
PLATO_INPUT_SEQUENCE_SUBTYPE(
(plato)(input_parser), ComponentAndTarget,
(plato::input_parser::IdentifierString, component)
(double, target)
)
// clang-format on

#endif
