#include "plato/input_parser/ComponentParserRegistration.hpp"

namespace plato::input_parser
{
auto registered_component_parsers() -> std::unordered_map<std::string, ComponentBlockParser>&
{
    static auto tRegisteredParsers = std::unordered_map<std::string, ComponentBlockParser>{};
    return tRegisteredParsers;
}
}  // namespace plato::input_parser
