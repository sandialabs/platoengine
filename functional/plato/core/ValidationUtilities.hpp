#ifndef PLATO_CORE_VALIDATIONUTILITIES
#define PLATO_CORE_VALIDATIONUTILITIES

#include <optional>
#include <string>
#include <vector>

#include "plato/core/InputVariantUtilities.hpp"
#include "plato/core/ValidationRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/utilities/ParameterBounds.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::core
{
/// @brief Calls the registered validation functions on all input blocks in @a aInput for
///  the input block types contained in @a InputVariant.
template <typename InputVariant>
[[nodiscard]] std::vector<std::string> validate_all_variants(const input_parser::ParsedInput& aInput,
                                                             std::vector<std::string>&& aCurrentMessageList);

template <typename InputVariant>
std::vector<std::string> validate_all_variants(const input_parser::ParsedInput& aInput,
                                               std::vector<std::string>&& aCurrentMessageList)
{
    const std::vector<InputVariant> tInputBlocks = core::all_input_blocks_in_variant<InputVariant>(aInput);
    for (const InputVariant& tBlockEntry : tInputBlocks)
    {
        aCurrentMessageList = std::visit(
            [tList = std::move(aCurrentMessageList)](const auto& aVariantInput) mutable -> std::vector<std::string>
            {
                return core::validate(aVariantInput, std::move(tList));  // NOLINT
            },
            tBlockEntry);
    }
    return std::move(aCurrentMessageList);
}

/// @brief Helper to get the raw input from a validated variant.
template <typename InputType, typename ValidatedVariant>
[[nodiscard]] const InputType& validated_variant_raw_input(const ValidatedVariant& aValidatedInput)
{
    assert(std::holds_alternative<ValidatedInputTypeWrapper<InputType>>(aValidatedInput.rawInput()));
    return std::get<ValidatedInputTypeWrapper<InputType>>(aValidatedInput.rawInput()).rawInput();
}

}  // namespace plato::core

#endif
