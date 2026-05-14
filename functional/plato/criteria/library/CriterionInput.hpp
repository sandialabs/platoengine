#ifndef PLATO_CRITERIA_LIBRARY_CRITERIONINPUT
#define PLATO_CRITERIA_LIBRARY_CRITERIONINPUT

#include <concepts>

#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"

namespace plato::criteria::library::detail
{
/// @brief Constrains templates to be instantiable only with either `input_parser::constraint` or
/// `input_parser::objective`.
template <typename Input>
concept CriterionInput = std::same_as<Input, input_parser::constraint> || std::same_as<Input, input_parser::objective>;
}  // namespace plato::criteria::library::detail

#endif
