#ifndef PLATO_PROCESSMANAGER_LIBRARY_VALIDATECROSSREFERENCES
#define PLATO_PROCESSMANAGER_LIBRARY_VALIDATECROSSREFERENCES

#include <optional>
#include <string>
#include <vector>

#include "plato/core/InputVariantUtilities.hpp"
#include "plato/core/VariantInputBuilder.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/process_manager/library/CrossReferenceUtilities.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::process_manager::library
{
/// @brief Validates cross reference fields in all input blocks in @a aInput.
template <typename FullInput>
[[nodiscard]] std::vector<std::string> validate_cross_referenced_input(FullInput& aInput,
                                                                       std::vector<std::string>&& aMessages);

namespace detail
{
template <typename VariantType, typename FieldType>
[[nodiscard]] std::optional<std::string> validate_expected_block(const std::vector<VariantType>& aAllLinkableBlocks,
                                                                 const FieldType& aField,
                                                                 const std::string& aBlockName)
{
    if (aAllLinkableBlocks.empty())
    {
        return utilities::concatenate(
            "Input block with name ", aBlockName,
            " has a cross-referenced field but no input blocks of the appropriate variant could be found.");
    }
    if (aAllLinkableBlocks.size() > 1 && !aField.has_value())
    {
        return utilities::concatenate(
            "Input block with name ", aBlockName,
            " has an ambiguous cross-reference. Multiple input blocks of the appropriate variant exist. Specify the "
            "desired input block to cross-reference by naming it in the cross-reference field.");
    }
    if (aField.has_value())
    {
        if (const auto& tNamedBlock = detail::find_cross_reference_named_block(aField.value(), aAllLinkableBlocks);
            tNamedBlock == aAllLinkableBlocks.cend())
        {
            return utilities::concatenate("Input block with name ", aBlockName,
                                          " has a cross-referenced field with name ", aField->mName,
                                          " but no input blocks matching this name were parsed.");
        }
    }
    return std::nullopt;
}

struct ValidateCrossReference
{
    std::vector<std::string>& mMessages;

    template <typename FieldType, typename InputBlock, typename FullInput>
    void operator()(FieldType& aField, const InputBlock&, const FullInput& aFullInput) const
    {
        using WrappedFieldType = typename plato::core::TypeOrOptional<std::decay_t<decltype(aField)>>::type;
        using VariantType = core::InputVariant<FullInput, WrappedFieldType::template IsVariantType>;
        const std::string tBlockName = plato::input_parser::InputTypeName<InputBlock>::name;
        const auto tAllLinkableBlocks = core::all_input_blocks_in_variant<VariantType>(aFullInput);
        std::optional<std::string> tMessage = validate_expected_block(tAllLinkableBlocks, aField, tBlockName);
        if (tMessage.has_value())
        {
            mMessages.emplace_back(std::move(tMessage).value());
        }
    }
};
}  // namespace detail

template <typename FullInput>
[[nodiscard]] std::vector<std::string> validate_cross_referenced_input(FullInput& aInput,
                                                                       std::vector<std::string>&& aMessages)
{
    apply_to_cross_references(aInput, detail::ValidateCrossReference{aMessages});
    return std::move(aMessages);
}
}  // namespace plato::process_manager::library

#endif