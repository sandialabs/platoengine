#ifndef PLATO_INPUT_PARSER_COMPONENTPARSERREGISTRATION
#define PLATO_INPUT_PARSER_COMPONENTPARSERREGISTRATION

#include "plato/input_parser/ComponentBlockParser.hpp"
#include "plato/input_parser/ComponentType.hpp"
#include "plato/input_parser/CrossLinker.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::input_parser
{
/// @brief Static registration class for registering a parser for a component.
///
/// To use, an object of this type must be instantiated with the struct that an input block is parsed to along with the
/// component type. Usage is:
/// @code{.cpp}
/// PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
///                         kernel_filter, plato::input_parser::ComponentType::kFilter,
///                         (double, filter_radius, "The filter radius"))
/// [[maybe_unused]] const auto kComponentRegistration = ComponentParserRegistration<kernel_filter>{};
/// @endcode
template <typename ComponentBlockType>
struct ComponentParserRegistration
{
    ComponentParserRegistration();
};

/// @brief Returns all registered parser block names corresponding to the ComponentType @a kComponentType.
///
/// This is mainly useful for error messages.
template <ComponentType kComponentType>
[[nodiscard]] auto all_block_names_with_component_type() -> std::vector<std::string>;

/// @brief Returns the full map of registered component parsers.
[[nodiscard]] auto registered_component_parsers() -> std::unordered_map<std::string, ComponentBlockParser>&;

/// @brief Returns the full set of CrossLinkers used to fill cross-references between input blocks.
[[nodiscard]] auto registered_cross_linkers() -> std::vector<CrossLinker>&;

template <typename ComponentBlockType>
ComponentParserRegistration<ComponentBlockType>::ComponentParserRegistration()
{
    registered_component_parsers().emplace(
        block_name<ComponentBlockType>(),
        make_component_block_parser<ComponentBlockType, ComponentTypeOfInputBlock<ComponentBlockType>::value>());
    registered_cross_linkers().push_back(make_cross_linker<ComponentBlockType>());
}

template <ComponentType kComponentType>
auto all_block_names_with_component_type() -> std::vector<std::string>
{
    const auto& tRegisteredParsers = registered_component_parsers();
    auto tParserNames = std::vector<std::string>{};
    utilities::transform_if(
        tRegisteredParsers, std::back_inserter(tParserNames),
        [](const auto& aNameAndParser) { return aNameAndParser.first; },
        [](const auto& aNameAndParser) { return aNameAndParser.second.componentType() == kComponentType; });
    return tParserNames;
}

}  // namespace plato::input_parser

#endif
