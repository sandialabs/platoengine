#ifndef PLATO_INPUT_PARSER_TEST_UTILITIES_PARSESINGLEINPUTBLOCK
#define PLATO_INPUT_PARSER_TEST_UTILITIES_PARSESINGLEINPUTBLOCK

#include <string>
#include <string_view>

#include "plato/input_parser/ComponentBlockParser.hpp"
#include "plato/input_parser/GenericBlockRule.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/utilities/Expected.hpp"

namespace plato::input_parser::test_utilities
{
/// @brief Attempts to parse @a aInput into a struct defined by @a InputBlock, which must be a struct with a parser
/// defined by PLATO_INPUT_BLOCK_STRUCT or PLATO_NAMED_INPUT_BLOCK_STRUCT.
template <input_parser::InputBlock InputBlock, components::ComponentType kComponentType>
[[nodiscard]] auto parse_single_input_block(std::string_view aInput) -> utilities::Expected<InputBlock, std::string>;

template <input_parser::InputBlock InputBlock, components::ComponentType kComponentType>
[[nodiscard]] auto parse_single_input_block(const std::string_view aInput)
    -> utilities::Expected<InputBlock, std::string>
{
    const auto tGenericBlockData = input_parser::parse_generic_blocks(aInput);
    if (tGenericBlockData.hasError())
    {
        return utilities::unexpected(tGenericBlockData.error());
    }

    const auto tConstraintParser =
        input_parser::ComponentBlockParser{InputBlock{}, input_parser::ComponentTypeHelper<kComponentType>{}};

    const auto tParsedResult = tConstraintParser.parse(tGenericBlockData.value().front());
    if (tParsedResult.hasError())
    {
        return utilities::unexpected(tParsedResult.error());
    }

    return tParsedResult.value().mInput.get<InputBlock>();
}

}  // namespace plato::input_parser::test_utilities

#endif
