#ifndef PLATO_GEOMETRY_EXTENSION_LEVELSETTOPOLOGYSPHEREPARSER
#define PLATO_GEOMETRY_EXTENSION_LEVELSETTOPOLOGYSPHEREPARSER

#include "plato/input_parser/AutoList.hpp"
#include "plato/input_parser/Point.hpp"
#include "plato/input_parser/SequenceSubtype.hpp"

/// @brief a LevelSetSphere is a void sphere that can be read in from the input
// clang-format off
// NOLINTBEGIN(cppcoreguidelines-special-member-functions)
PLATO_INPUT_SEQUENCE_SUBTYPE(
(plato)(input_parser),LevelSetSphere,
(double, radius)
(plato::input_parser::Point, center)
)
// NOLINTEND(cppcoreguidelines-special-member-functions)
// clang-format on

namespace plato::input_parser
{

[[nodiscard]] constexpr inline auto operator==(const LevelSetSphere& aLHS, const LevelSetSphere& aRHS) -> bool
{
    return aLHS.radius == aRHS.radius && aLHS.center == aRHS.center;
}

using LevelSetSphereList = AutoList<LevelSetSphere>;
}  // namespace plato::input_parser

#endif
