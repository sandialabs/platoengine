#include "plato/input_parser/InputFieldTypes.hpp"

#include <string>

namespace plato::input_parser
{
auto data_type_documentation(const AppName& /*unused*/) -> std::string { return "String"; }

auto data_type_documentation(const BlockList& /*unused*/) -> std::string { return "Comma separated list of strings"; }

auto data_type_documentation(const IdentifierString& /*unused*/) -> std::string { return "String"; }
}  // namespace plato::input_parser
