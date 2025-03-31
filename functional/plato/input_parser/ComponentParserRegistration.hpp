#ifndef PLATO_INPUT_PARSER_COMPONENTPARSERREGISTRATION
#define PLATO_INPUT_PARSER_COMPONENTPARSERREGISTRATION

#include "plato/input_parser/ComponentBlockParser.hpp"
#include "plato/input_parser/ComponentType.hpp"

namespace plato::input_parser
{
/// @brief Static registration class for registering a parser for a component.
///
/// To use, an object of this type must be instantiated with the struct that an input block is parsed to along with the
/// component type. Usage is:
/// @code{.cpp}
/// PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
///                         kernel_filter,
///                         (double, filter_radius, "The filter radius"))
/// [[maybe_unused]] const auto kComponentRegistration = ComponentParserRegistration<kernel_filter,
/// input_parser::ComponentType::kFilter>{}
/// @endcode
template <typename ComponentBlockType, ComponentType kComponentType>
struct ComponentParserRegistration
{
    ComponentParserRegistration();
};

/// @brief Returns the full map of registered component parsers.
auto registered_component_parsers() -> std::unordered_map<std::string, ComponentBlockParser>&;

template <typename ComponentBlockType, ComponentType kComponentType>
ComponentParserRegistration<ComponentBlockType, kComponentType>::ComponentParserRegistration()
{
    registered_component_parsers().emplace(block_name<ComponentBlockType>(),
                                           make_component_block_parser<ComponentBlockType, kComponentType>());
}

}  // namespace plato::input_parser

#endif
