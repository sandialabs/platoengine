#ifndef PLATO_FILTER_LIBRARY_FILTERVALIDATION
#define PLATO_FILTER_LIBRARY_FILTERVALIDATION

#include <optional>
#include <string>

namespace plato::input_parser
{
struct density_topology;
}

namespace plato::filter::library
{
[[nodiscard]] std::optional<std::string> validate_filter_type(const input_parser::density_topology& aInput);
}  // namespace plato::filter::library

#endif
