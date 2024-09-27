#ifndef PLATO_FILTER_EXTENSION_HELMHOLTZFILTER
#define PLATO_FILTER_EXTENSION_HELMHOLTZFILTER

#include <optional>
#include <string>

#include "plato/filter/library/FilterRegistration.hpp"

namespace plato::input_parser
{
struct helmholtz_filter;
}
namespace plato::filter::extension
{
[[nodiscard]] std::optional<std::string> validate_helmholtz_filter_boundary_sticking_penalty(
    const input_parser::helmholtz_filter& aInput);

namespace detail
{
/// @brief Create a StateCache object for constructing a shared pointer to a FilterInterface loaded from a shared
/// library for Helmholtz filter if the mesh coordinates have changed (i.e. the mesh has changed)
[[nodiscard]] library::FilterCache create_filter_cache(const input_parser::helmholtz_filter& aInput);
}  // namespace detail
}  // namespace plato::filter::extension

#endif
