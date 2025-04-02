#ifndef PLATO_INPUT_PARSER_CROSSLINKER
#define PLATO_INPUT_PARSER_CROSSLINKER

#include <functional>

#include "plato/core/VariantInputBuilder.hpp"
#include "plato/input_parser/ComponentBlockParser.hpp"
#include "plato/input_parser/ParsedInput.hpp"

namespace plato::input_parser
{
/// @brief Template for deducing the input type without instantiating an object.
template <typename Input>
struct InputTypeHelper
{
};

/// @brief A type trait specifying if a type is a NewCrossReference templated on @a T
template <typename T>
constexpr bool kIsNewCrossReference = false;

template <ComponentType kComponentType>
constexpr bool kIsNewCrossReference<NewCrossReference<kComponentType>> = true;

/// @brief This class provides a method for filling cross-references in input blocks.
class CrossLinker
{
   public:
    template <typename Input>
    CrossLinker(InputTypeHelper<Input>);

    /// @brief Fills the cross-reference field in @a aInputBlock with the input contained in @a aNewParsedInput.
    void crossLink(InputDataBlock& aInputBlock, const NewParsedInput& aNewParsedInput) const;

   private:
    std::function<void(InputDataBlock&, const NewParsedInput&)> mCrossLinkFunction;
};

/// @brief Helper function for constructing a CrossLinker for a given input type.
template <typename Input>
[[nodiscard]] auto make_cross_linker() -> CrossLinker;

template <typename Input>
auto make_cross_linker() -> CrossLinker
{
    return CrossLinker{InputTypeHelper<Input>{}};
}

namespace detail
{
template <typename ComponentInput>
void apply_cross_link(ComponentInput& aComponent, const std::vector<InputDataBlock>& aAllComponentsOfMatchingType)
{
    assert(!aAllComponentsOfMatchingType.empty());
    if (!aComponent.has_value())
    {
        // Component to link was not specified, use the first in the list
        aComponent.emplace();
        aComponent->mName = aAllComponentsOfMatchingType.front().mBlockName;
        aComponent->mInputBlock = aAllComponentsOfMatchingType.front().mInput;
    }
    else
    {
        // Component to link was specified, find the input with the matching name
        const auto tCrossReferenceBlock =
            std::find_if(aAllComponentsOfMatchingType.begin(), aAllComponentsOfMatchingType.end(),
                         [&aComponent](const auto& tComponent) { return tComponent.mBlockName == aComponent->mName; });
        assert(tCrossReferenceBlock != aAllComponentsOfMatchingType.end());
        aComponent->mInputBlock = tCrossReferenceBlock->mInput;
    }
}

template <typename Input>
void cross_link(InputDataBlock& aInputBlock, const NewParsedInput& aNewParsedInput)
{
    assert(aInputBlock.mInput.holds_expected_type<Input>());
    auto& aCastInputBlock = aInputBlock.mInput.get<Input&>();
    boost::fusion::for_each(aCastInputBlock,
                            [&aNewParsedInput](auto& aField)
                            {
                                using FieldType = typename core::TypeOrOptional<std::decay_t<decltype(aField)>>::type;
                                if constexpr (kIsNewCrossReference<FieldType>)
                                {
                                    apply_cross_link(aField, aNewParsedInput.get<FieldType::mComponentType>());
                                }
                            });
}
}  // namespace detail

template <typename Input>
CrossLinker::CrossLinker(InputTypeHelper<Input>)
    : mCrossLinkFunction{[](InputDataBlock& aInputBlock, const NewParsedInput& aNewParsedInput)
                         {
                             if (aInputBlock.mInput.holds_expected_type<Input>())
                             {
                                 detail::cross_link<Input>(aInputBlock, aNewParsedInput);
                             }
                         }}
{
}
}  // namespace plato::input_parser

#endif
