#ifndef PLATO_FILTER_EXTENSION_HELMHOLTZFILTER
#define PLATO_FILTER_EXTENSION_HELMHOLTZFILTER

#include <optional>
#include <string>

#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"

// clang-format off
PLATO_FILTER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser),
    new_helmholtz_filter,
    (double, filter_radius, "Required field specifying the size of the filter radius.")
    (bool, use_relative_radius, "Optional field that can convert the filter radius specified into a radius relative to the average element size.")
    (double, boundary_sticking_penalty, "Optional field specifying the penalization of material that sticks to boundaries of the design "
                                        "domain as an artifact of filtering. Unit bounded: a value of 1 applies maximum penalization."))
// clang-format on

namespace plato::filter::extension
{
/// @brief Creates a valid example input struct for a HelmholtzFilter, useful for testing.
[[nodiscard]] auto create_valid_helmholtz_filter_input() -> input_parser::new_helmholtz_filter;

/// @brief Checks that the boundary_sticking_penalty in @a aInput contains a valid value.
[[nodiscard]] auto validate_helmholtz_filter_boundary_sticking_penalty(const input_parser::new_helmholtz_filter& aInput)
    -> std::optional<std::string>;

namespace detail
{
/// @brief Create a StateCache object for constructing a shared pointer to a FilterInterface loaded from a shared
/// library for Helmholtz filter if the mesh coordinates have changed (i.e. the mesh has changed)
[[nodiscard]] auto create_filter_cache(const input_parser::new_helmholtz_filter& aInput) -> library::FilterCache;
}  // namespace detail
}  // namespace plato::filter::extension

#endif
