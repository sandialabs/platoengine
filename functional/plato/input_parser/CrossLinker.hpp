#ifndef PLATO_INPUT_PARSER_CROSSLINKER
#define PLATO_INPUT_PARSER_CROSSLINKER

#include <functional>

#include "plato/input_parser/ComponentBlockParser.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/utilities/Expected.hpp"
#include "plato/utilities/StringUtilities.hpp"

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
    using CrossLinkedBlockOrError = utilities::Expected<InputDataBlock, std::string>;

    template <typename Input>
    CrossLinker(InputTypeHelper<Input>);

    /// @brief Fills the cross-reference field in @a aInputBlock with the input contained in @a aNewParsedInput.
    [[nodiscard]] auto crossLink(InputDataBlock aInputBlock, const NewParsedInput& aNewParsedInput) const
        -> CrossLinkedBlockOrError;

   private:
    std::function<CrossLinkedBlockOrError(InputDataBlock, const NewParsedInput&)> mCrossLinkFunction;
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
struct ErrorCheckCrossLink
{
    std::string mInputBlockName;
    std::vector<std::string> mErrorMessages;

    auto blockNames(const std::vector<InputDataBlock>& aAllComponentsOfMatchingType) -> std::string
    {
        auto tBlockNames = std::vector<std::string>{};
        std::transform(aAllComponentsOfMatchingType.begin(), aAllComponentsOfMatchingType.end(),
                       std::back_inserter(tBlockNames), [](const auto& aInputBlock) { return aInputBlock.mBlockName; });
        return utilities::concatenate_container(tBlockNames, ", ");
    }

    template <typename ComponentInput>
    void operator()(const ComponentInput& aComponentToLink,
                    const std::vector<InputDataBlock>& aAllComponentsOfMatchingType)
    {
        const auto tErrorPreamble =
            utilities::concatenate("Error satisfying cross-reference for ", mInputBlockName, ": ");
        if (!aComponentToLink.has_value() && aAllComponentsOfMatchingType.size() > 1U)
        {
            mErrorMessages.push_back(utilities::concatenate(tErrorPreamble,
                                                            "Unspecified cross-reference requires that only one "
                                                            "matching component be defined. Found the following: ",
                                                            blockNames(aAllComponentsOfMatchingType)));
        }

        if (!aComponentToLink.has_value() && aAllComponentsOfMatchingType.empty())
        {
            mErrorMessages.push_back(utilities::concatenate(
                tErrorPreamble, "No components are defined that match the cross-reference's required component type"));
        }

        const auto tComponentsMatch = [&aComponentToLink](const auto& tComponent)
        { return tComponent.mBlockName == aComponentToLink->mName; };

        if (aComponentToLink.has_value() &&
            std::find_if(aAllComponentsOfMatchingType.begin(), aAllComponentsOfMatchingType.end(), tComponentsMatch) ==
                aAllComponentsOfMatchingType.end())
        {
            mErrorMessages.push_back(utilities::concatenate(
                tErrorPreamble, "Specified cross-reference (", aComponentToLink->mName,
                ") was not found in the input. The following may be used: ", blockNames(aAllComponentsOfMatchingType)));
        }
    }
};

struct ApplyCrossLink
{
    template <typename ComponentInput>
    void operator()(ComponentInput& aComponentToLink, const std::vector<InputDataBlock>& aAllComponentsOfMatchingType)
    {
        assert(!aAllComponentsOfMatchingType.empty());
        if (!aComponentToLink.has_value())
        {
            assert(aAllComponentsOfMatchingType.size() == 1U);
            // Component to link was not specified, use the first in the list
            aComponentToLink.emplace();
            aComponentToLink->mName = aAllComponentsOfMatchingType.front().mBlockName;
            aComponentToLink->mInputBlock = aAllComponentsOfMatchingType.front().mInput;
        }
        else
        {
            // Component to link was specified, find the input with the matching name
            const auto tCrossReferenceBlock =
                std::find_if(aAllComponentsOfMatchingType.begin(), aAllComponentsOfMatchingType.end(),
                             [&aComponentToLink](const auto& tComponent)
                             { return tComponent.mBlockName == aComponentToLink->mName; });
            assert(tCrossReferenceBlock != aAllComponentsOfMatchingType.end());
            aComponentToLink->mInputBlock = tCrossReferenceBlock->mInput;
        }
    }
};

template <typename T>
struct TypeOrOptional
{
    using type = T;
};

template <typename T>
struct TypeOrOptional<boost::optional<T>>
{
    using type = T;
};

template <typename Input, typename ApplyFunction>
void apply_cross_link(InputDataBlock& aInputBlock, const NewParsedInput& aNewParsedInput, ApplyFunction& aApplyFunction)
{
    assert(aInputBlock.mInput.holds_expected_type<Input>());
    auto& aCastInputBlock = aInputBlock.mInput.get<Input&>();
    boost::fusion::for_each(aCastInputBlock,
                            [&aNewParsedInput, &aApplyFunction](auto& aField)
                            {
                                using FieldType = typename TypeOrOptional<std::decay_t<decltype(aField)>>::type;
                                if constexpr (kIsNewCrossReference<FieldType>)
                                {
                                    aApplyFunction(aField, aNewParsedInput.get<FieldType::mComponentType>());
                                }
                            });
}
}  // namespace detail

template <typename Input>
CrossLinker::CrossLinker(InputTypeHelper<Input>)
    : mCrossLinkFunction{
          [](InputDataBlock aInputBlock, const NewParsedInput& aNewParsedInput) -> CrossLinkedBlockOrError
          {
              if (aInputBlock.mInput.holds_expected_type<Input>())
              {
                  auto tErrorChecks = detail::ErrorCheckCrossLink{aInputBlock.mBlockName, {}};
                  detail::apply_cross_link<Input>(aInputBlock, aNewParsedInput, tErrorChecks);
                  if (tErrorChecks.mErrorMessages.empty())
                  {
                      auto tApplyCrossLink = detail::ApplyCrossLink{};
                      detail::apply_cross_link<Input>(aInputBlock, aNewParsedInput, tApplyCrossLink);
                      return aInputBlock;
                  }
                  return utilities::unexpected(utilities::concatenate_container(tErrorChecks.mErrorMessages, "\n"));
              }
              return aInputBlock;
          }}
{
}
}  // namespace plato::input_parser

#endif
