#ifndef PLATO_CRITERIA_LIBRARY_CONSTRAINTVALIDATION
#define PLATO_CRITERIA_LIBRARY_CONSTRAINTVALIDATION

#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/input_parser/InputBlocks.hpp"

namespace plato::criteria::library
{
/// @brief Validates all constraint inputs in @a aInput, returning all error messages and appending to @a
/// aCurrentMessageList.
[[nodiscard]] auto validate_constraints(const std::vector<input_parser::constraint>& aInput,
                                        std::vector<std::string>&& aCurrentMessageList) -> std::vector<std::string>;

namespace detail
{
[[nodiscard]] auto validate_constraint_value(const input_parser::new_constraint& aInput) -> std::optional<std::string>;
[[nodiscard]] auto validate_constraint_number_of_processors(const input_parser::new_constraint& aInput)
    -> std::optional<std::string>;

[[nodiscard]] auto validate_constraint_type(const input_parser::new_constraint& aInput) -> std::optional<std::string>;
}  // namespace detail

}  // namespace plato::criteria::library

#endif
