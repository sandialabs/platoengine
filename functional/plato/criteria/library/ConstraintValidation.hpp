#ifndef PLATO_CRITERIA_LIBRARY_CONSTRAINTVALIDATION
#define PLATO_CRITERIA_LIBRARY_CONSTRAINTVALIDATION

#include "plato/core/ValidationRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"

namespace plato::criteria::library
{
namespace detail
{
[[nodiscard]] std::optional<std::string> validate_equal_to(const input_parser::constraint& aInput);

}  // namespace detail

[[nodiscard]] std::vector<std::string> validate_constraints(const std::vector<input_parser::constraint>& aInput,
                                                            std::vector<std::string>&& aCurrentMessageList);

}  // namespace plato::criteria::library

#endif