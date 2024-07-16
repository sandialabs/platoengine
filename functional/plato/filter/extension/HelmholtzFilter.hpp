#ifndef PLATO_FILTER_EXTENSION_HELMHOLTZFILTER
#define PLATO_FILTER_EXTENSION_HELMHOLTZFILTER

#include <optional>
#include <string>

namespace plato::input_parser
{
struct helmholtz_filter;
}
namespace plato::filter::extension
{
[[nodiscard]] std::optional<std::string> validate_helmholtz_filter_boundary_sticking_penalty(
    const input_parser::helmholtz_filter& aInput);

}  // namespace plato::filter::extension

#endif
