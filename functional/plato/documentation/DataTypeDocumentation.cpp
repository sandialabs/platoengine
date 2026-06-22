#include "plato/documentation/DataTypeDocumentation.hpp"

namespace plato::documentation
{
auto data_type_documentation(const double /*unused*/) -> std::string { return "Floating point"; };

auto data_type_documentation(const int /*unused*/) -> std::string { return "Signed integer"; };

auto data_type_documentation(const unsigned int /*unused*/) -> std::string { return "Unsigned integer"; };

auto data_type_documentation(const bool /*unused*/) -> std::string { return "`true` or `false`"; };

}  // namespace plato::documentation
