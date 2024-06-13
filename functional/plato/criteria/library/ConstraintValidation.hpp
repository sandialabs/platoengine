#ifndef PLATO_CRITERIA_LIBRARY_CONSTRAINTVALIDATION
#define PLATO_CRITERIA_LIBRARY_CONSTRAINTVALIDATION

#include "plato/core/ValidationRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"

namespace plato::criteria::library
{
/// @brief Validates all constraint inputs in @a aInput, returning all error messages and appending to @a
/// aCurrentMessageList.
[[nodiscard]] std::vector<std::string> validate_constraints(const std::vector<input_parser::constraint>& aInput,
                                                            std::vector<std::string>&& aCurrentMessageList);

namespace detail
{
[[nodiscard]] std::optional<std::string> validate_equal_to(const input_parser::constraint& aInput);
[[nodiscard]] std::optional<std::string> validate_constraint_number_of_processors(
    const input_parser::constraint& aInput);
}  // namespace detail

}  // namespace plato::criteria::library

#endif
