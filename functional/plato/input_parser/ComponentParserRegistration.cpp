#include "plato/input_parser/ComponentParserRegistration.hpp"

namespace plato::input_parser
{
auto registered_component_parsers() -> std::unordered_map<std::string, ComponentBlockParser>&
{
    static auto tRegisteredParsers = std::unordered_map<std::string, ComponentBlockParser>{};
    return tRegisteredParsers;
}

auto registered_cross_linkers() -> std::vector<CrossLinker>&
{
    static auto tRegisteredCrossLinkers = std::vector<CrossLinker>{};
    return tRegisteredCrossLinkers;
}

}  // namespace plato::input_parser
