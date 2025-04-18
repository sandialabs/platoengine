#ifndef PLATO_CRITERIA_LIBRARY_CONSTRAINTVALIDATION
#define PLATO_CRITERIA_LIBRARY_CONSTRAINTVALIDATION

#include "plato/criteria/library/ConstraintInputBlock.hpp"

namespace plato::criteria::library::detail
{
[[nodiscard]] auto validate_constraint_value(const input_parser::constraint& aInput) -> std::optional<std::string>;

[[nodiscard]] auto validate_constraint_number_of_processors(const input_parser::constraint& aInput)
    -> std::optional<std::string>;

[[nodiscard]] auto validate_constraint_type(const input_parser::constraint& aInput) -> std::optional<std::string>;
}  // namespace plato::criteria::library::detail

#endif
