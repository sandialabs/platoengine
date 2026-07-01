#include "plato/documentation/DataTypeDocumentation.hpp"

#include <string>

namespace plato::documentation
{
auto data_type_documentation(const double /*unused*/) -> std::string { return "`<real>`"; };

auto data_type_documentation(const int /*unused*/) -> std::string { return "`<int>`"; };

auto data_type_documentation(const unsigned int /*unused*/) -> std::string { return "`<unsigned int>`"; };

auto data_type_documentation(const bool /*unused*/) -> std::string { return "`true | false`"; };

}  // namespace plato::documentation
