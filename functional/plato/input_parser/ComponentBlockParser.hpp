#ifndef PLATO_INPUT_PARSER_COMPONENTBLOCKPARSER
#define PLATO_INPUT_PARSER_COMPONENTBLOCKPARSER

#include <any>
#include <string>

#include "plato/input_parser/BlockStructRule.hpp"
#include "plato/input_parser/GenericBlockRule.hpp"

namespace plato::input_parser
{

/// @brief Type-erased wrapper for holding the parsed data of a component input block.
struct InputDataBlock
{
    ComponentType mComponentType;
    std::string mBlockName;
    CrossReferencedInput mInput;
};

/// @brief A parser that parses a generic component block using the component's specific rule to parse into a struct.
///
/// The body of a component block contained in a GenericBlockData object is parsed into a struct as provided to the
/// constructor, and the result is held in a type-erased wrapper, InputDataBlock.
class ComponentBlockParser
{
   public:
    template <typename Parser>
    ComponentBlockParser(Parser aParser);

    /// @brief Parses @a aData into the struct provided to the constructor.
    ///
    /// The result is stored in type-erased wrapper, and so the type of the struct must be known to retrieve the data.
    [[nodiscard]] auto parse(const GenericBlockData& aData) const -> InputDataBlock;

   private:
    std::function<InputDataBlock(const std::any&, const GenericBlockData&)> mParseFunction;
    std::any mParserObject;
};

template <typename Parser>
ComponentBlockParser::ComponentBlockParser(Parser aParser)
    : mParseFunction{[](const std::any& aParserObject, const GenericBlockData& aData) -> InputDataBlock
                     {
                         const auto& tParser = std::any_cast<const Parser&>(aParserObject);
                         const auto tInput = to_string(aData);
                         auto tInputIterator = tInput.begin();
                         auto tData = typename Parser::BlockDataStruct{};
                         const auto tSkipper = SkipperRule<std::string::const_iterator>{};
                         phrase_parse(tInputIterator, tInput.cend(), tParser.mBlockRule, tSkipper.skipperRule(), tData);
                         auto tWrappedResult = InputDataBlock{};
                         tWrappedResult.mInput.set(tData);
                         tWrappedResult.mBlockName = aData.mName.mToken;
                         tWrappedResult.mComponentType = Parser::mComponentType;
                         return tWrappedResult;
                     }},
      mParserObject{std::move(aParser)}
{
}

auto ComponentBlockParser::parse(const GenericBlockData& aData) const -> InputDataBlock
{
    return mParseFunction(mParserObject, aData);
}

}  // namespace plato::input_parser

#endif
