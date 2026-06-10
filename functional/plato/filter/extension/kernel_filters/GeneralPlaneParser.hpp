#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_GENERALPLANEPARSER
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_GENERALPLANEPARSER

#include "plato/input_parser/Point.hpp"
#include "plato/input_parser/SequenceSubtype.hpp"

///@brief Input struct with parser to read in a generalized plane the input deck with a center and normal
// clang-format off
PLATO_INPUT_SEQUENCE_SUBTYPE(
(plato)(input_parser), GeneralPlane,
(plato::input_parser::Point, center)
(plato::input_parser::Point, normal)
)
// clang-format on

#endif
