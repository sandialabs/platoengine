#ifndef PLATO_INPUT_PARSER_CROSSLINKEDINPUT
#define PLATO_INPUT_PARSER_CROSSLINKEDINPUT

#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_parser/CrossLinker.hpp"
#include "plato/utilities/Expected.hpp"

namespace plato::input_parser
{
class CrossLinkedInput;

/// @brief Implementation of the pass-key idiom for limiting construction of CrossLinkedInput.
struct CrossLinkKey
{
    friend auto make_cross_linked_input(input_parser::ParsedInput input)
        -> utilities::Expected<CrossLinkedInput, std::string>;

   private:
    CrossLinkKey() {}
    CrossLinkKey(const CrossLinkKey&) {}
};

/// @brief A class that takes @a ParsedInput and links cross references for use in validation.
class CrossLinkedInput
{
   public:
    CrossLinkedInput(input_parser::ParsedInput aInput, const CrossLinkKey&);

    [[nodiscard]] auto rawInput() const -> const input_parser::ParsedInput&;

   private:
    input_parser::ParsedInput mInput;
};

/// @brief friend function to construct a @a CrossLinkedInput object from the @a ParsedInput @param aInput. Uses the
/// passkey idiom to ensure that only this function can construct a @a CrossLinkedInput
[[nodiscard]] auto make_cross_linked_input(input_parser::ParsedInput aInput)
    -> utilities::Expected<CrossLinkedInput, std::string>;

}  // namespace plato::input_parser

#endif
