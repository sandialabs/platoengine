#ifndef PLATO_INPUT_PARSER_COMPONENTBLOCKPARSER
#define PLATO_INPUT_PARSER_COMPONENTBLOCKPARSER

#include <any>
#include <string>

#include "plato/input_parser/BlockStructRule.hpp"
#include "plato/input_parser/GenericBlockRule.hpp"
#include "plato/input_parser/ParseErrorUtilities.hpp"
#include "plato/utilities/Expected.hpp"

namespace plato::input_parser
{

/// @brief Type-erased wrapper for holding the parsed data of a component input block.
struct InputDataBlock
{
    ComponentType mComponentType;
    std::string mBlockName;
    CrossReferencedInput mInput;
};

template <ComponentType kComponentType>
struct ComponentTypeHelper
{
};

/// @brief A parser that parses a generic component block using the component's specific rule to parse into a struct.
///
/// The body of a component block contained in a GenericBlockData object is parsed into a struct as provided to the
/// constructor, and the result is held in a type-erased wrapper, InputDataBlock.
class ComponentBlockParser
{
   public:
    using ParsedDataOrError = utilities::Expected<InputDataBlock, std::string>;

    template <typename InputType, ComponentType kComponentType>
    ComponentBlockParser(const InputType&, ComponentTypeHelper<kComponentType>);

    /// @brief Parses @a aData into the struct provided to the constructor.
    ///
    /// The result is stored in type-erased wrapper, and so the type of the struct must be known to retrieve the data.
    [[nodiscard]] auto parse(const GenericBlockData& aData) const -> ParsedDataOrError;

    /// @brief Returns the ComponentType of the parser.
    [[nodiscard]] auto componentType() const -> ComponentType;

   private:
    ComponentType mComponentType;
    std::function<ParsedDataOrError(const GenericBlockData&)> mParseFunction;
};

/// @brief Helper function for constructing a ComponentBlockParser
template <typename InputType, ComponentType kComponentType>
[[nodiscard]] auto make_component_block_parser() -> ComponentBlockParser;

template <typename InputType, ComponentType kComponentType>
ComponentBlockParser::ComponentBlockParser(const InputType&, ComponentTypeHelper<kComponentType>)
    : mComponentType{kComponentType},
      mParseFunction{
          [](const GenericBlockData& aData) -> ParsedDataOrError
          {
              using Parser = ComponentBlockRule<std::string::const_iterator, InputType, kComponentType>;
              const auto tParser = Parser{};
              const auto tInput = to_string(aData);
              auto tInputIterator = tInput.begin();
              auto tData = typename Parser::BlockDataStruct{};
              const auto tSkipper = SkipperRule<std::string::const_iterator>{};
              const auto tParsedSuccessfully = boost::spirit::qi::phrase_parse(
                  tInputIterator, tInput.cend(), tParser.mBlockRule, tSkipper.skipperRule(), tData);

              if (parser_has_error(tParsedSuccessfully, tInputIterator, tInput.cend()))
              {
                  constexpr auto tDelimeter = ' ';
                  return utilities::unexpected(error_message(tInputIterator, tInput.cend(), tDelimeter));
              }

              return InputDataBlock{Parser::mComponentType, aData.mName.mToken, CrossReferencedInput{std::move(tData)}};
          }}
{
}

template <typename InputType, ComponentType kComponentType>
auto make_component_block_parser() -> ComponentBlockParser
{
    return ComponentBlockParser{InputType{}, ComponentTypeHelper<kComponentType>{}};
}

}  // namespace plato::input_parser

#endif
