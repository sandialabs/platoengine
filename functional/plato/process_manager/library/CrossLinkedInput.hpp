#ifndef PLATO_PROCESSMANAGER_LIBRARY_CROSSLINKEDINPUT
#define PLATO_PROCESSMANAGER_LIBRARY_CROSSLINKEDINPUT

#include "plato/process_manager/library/CrossReferenceUtilities.hpp"

namespace plato::process_manager::library
{
class CrossLinkedInput;

struct CrossLinkKey
{
    friend CrossLinkedInput make_cross_linked_input(input_parser::ParsedInput input);

   private:
    CrossLinkKey() {}
    CrossLinkKey(const CrossLinkKey&) {}
};

/// @brief A class that takes @a ParsedInput and links cross references for use in validation.
class CrossLinkedInput
{
   public:
    CrossLinkedInput(input_parser::ParsedInput aInput, const CrossLinkKey&);

    const input_parser::ParsedInput& rawInput() const { return mInput; };

   private:
    template <typename FieldType, typename InputBlock, typename FullInput>
    static void fillCrossReference(FieldType& aField, const InputBlock&, const FullInput& aFullInput);

   private:
    input_parser::ParsedInput mInput;
};

/// @brief friend function to construct a @a CrossLinkedInput object from the @a ParsedInput @param aInput. Uses the
/// passkey idiom to ensure that only this function can construct a @a CrossLinkedInput
[[nodiscard]] CrossLinkedInput make_cross_linked_input(input_parser::ParsedInput aInput);

template <typename FieldType, typename InputBlock, typename FullInput>
void CrossLinkedInput::fillCrossReference(FieldType& aField, const InputBlock&, const FullInput& aFullInput)
{
    using WrappedFieldType = typename plato::core::TypeOrOptional<std::decay_t<decltype(aField)>>::type;
    using VariantType = core::InputVariant<FullInput, WrappedFieldType::template IsVariantType>;
    if (!aField.has_value())
    {
        aField.emplace();
        const auto aFirstInputBlock = core::first_input_block_in_variant<VariantType>(aFullInput);
        assert(aFirstInputBlock.has_value());
        aField->mInputBlock.set(aFirstInputBlock.value());  // NOLINT
    }
    else
    {
        const auto tAllLinkableBlocks = core::all_input_blocks_in_variant<VariantType>(aFullInput);
        const auto& tNamedBlock = detail::find_cross_reference_named_block(aField.value(), tAllLinkableBlocks);
        assert(tNamedBlock != tAllLinkableBlocks.cend());
        aField->mInputBlock.set(*tNamedBlock);
    }
}

}  // namespace plato::process_manager::library

#endif
