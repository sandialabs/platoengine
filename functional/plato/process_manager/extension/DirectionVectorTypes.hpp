#ifndef PLATO_PROCESSMANAGER_EXTENSION_DIRECTIONVECTORTYPES
#define PLATO_PROCESSMANAGER_EXTENSION_DIRECTIONVECTORTYPES

#include "plato/input_parser/EnumTypeHelpers.hpp"

namespace ROL
{
template <typename T>
class Vector;
}

// clang-format off
DECLARE_ENUM_AND_ENUM_SYMBOL_TABLE(DirectionVectorTypes,
                                   plato::input_parser,
                                   (kRandom, "random")
                                   (kUniformPositive, "uniform_positive")
                                   (kUniformNegative, "uniform_negative")
                                   )
// clang-format on

namespace plato::process_manager::extension
{
/// @brief Take a properly sized rol vector @a aROLVector and randomize it with @a aRandomDirectionSeed if the optional
/// has a value or make it all uniform 1s if it is nullopt.
void make_uniform_or_random_perturbation(ROL::Vector<double>& aROLVector,
                                         const input_parser::DirectionVectorTypes aDirectionVectorType);
}  // namespace plato::process_manager::extension
#endif
