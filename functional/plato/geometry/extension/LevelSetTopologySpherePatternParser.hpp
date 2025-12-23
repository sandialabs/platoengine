#ifndef PLATO_GEOMETRY_EXTENSION_LEVELSETSPHEREPATTERNPARSER
#define PLATO_GEOMETRY_EXTENSION_LEVELSETSPHEREPATTERNPARSER

#include "plato/input_parser/Point.hpp"
#include "plato/input_parser/SequenceSubtype.hpp"

///@brief Input struct with parser to read in inclusive bounds from the input deck
// clang-format off
PLATO_INPUT_SEQUENCE_SUBTYPE(
(plato)(input_parser),SpherePattern,
(double, radius)
(double, spacing)
(plato::input_parser::Point, min)
(plato::input_parser::Point, max)
)
// clang-format on

#endif
