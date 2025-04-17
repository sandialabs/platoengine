#ifndef PLATO_INPUT_PARSER_PARSEDINPUT
#define PLATO_INPUT_PARSER_PARSEDINPUT

#include <array>
#include <unordered_map>
#include <vector>

#include "plato/input_parser/BlockStructRule.hpp"
#include "plato/input_parser/ComponentBlockParser.hpp"
#include "plato/utilities/EnumIndexing.hpp"
#include "plato/utilities/Expected.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::input_parser
{
/// @brief The in-memory representation of a parsed input deck.
///
/// This class contains the result of a parsed input deck. The input is organized into vectors that are retrievable with
/// the component type.
/// @note The input stored in this class has not been validated and so is not guaranteed to have the correct number of
/// components for any component type.
/// @todo Fix class name
class NewParsedInput
{
   public:
    NewParsedInput() = default;

    /// @brief @a aRawInput is the result of parsing an input deck without organizing the results by component type.
    NewParsedInput(std::vector<InputDataBlock> aRawInput);

    /// @brief Returns the parsed input blocks corresponding to @a kComponentType
    template <ComponentType kComponentType>
    [[nodiscard]] auto get() const -> const std::vector<InputDataBlock>&;

    /// @brief Returns the parsed input blocks corresponding to @a kComponentType
    template <ComponentType kComponentType>
    [[nodiscard]] auto get() -> std::vector<InputDataBlock>&;

    /// @brief Returns the parsed input blocks corresponding to the type @a InputType
    ///
    /// This may be used to retrieve a specific input block.
    template <typename InputType>
    [[nodiscard]] auto get() const -> std::vector<InputType>;

   private:
    static constexpr inline std::size_t kNumberOfComponents = utilities::number_of_enumerates<ComponentType>();
    std::array<std::vector<InputDataBlock>, kNumberOfComponents> mInputBlocks;
};

/// @brief Parse the string @a aInput to a NewParsedInput object using the registered parsers.
/// @todo Fix name
[[nodiscard]] auto parse_to_new_input(const std::string& aInput) -> utilities::Expected<NewParsedInput, std::string>;

/// @brief Parse the string @a aInput to a NewParsedInput object.
/// @todo Fix name
[[nodiscard]] auto parse_to_new_input(const std::string& aInput,
                                      const std::unordered_map<std::string, ComponentBlockParser>& aComponentParsers)
    -> utilities::Expected<NewParsedInput, std::string>;

template <ComponentType kComponentType>
auto NewParsedInput::get() -> std::vector<InputDataBlock>&
{
    static_assert(kComponentType != ComponentType::kNumberOfEnumerates,
                  "ParsedInput::get must only be instantiated with a valid ComponentType.");
    return std::get<utilities::enum_index(kComponentType)>(mInputBlocks);
}

template <ComponentType kComponentType>
auto NewParsedInput::get() const -> const std::vector<InputDataBlock>&
{
    static_assert(kComponentType != ComponentType::kNumberOfEnumerates,
                  "ParsedInput::get must only be instantiated with a valid ComponentType.");
    return std::get<utilities::enum_index(kComponentType)>(mInputBlocks);
}

template <typename InputType>
auto NewParsedInput::get() const -> std::vector<InputType>
{
    constexpr auto tComponentType = ComponentTypeOfInputBlock<InputType>::value;
    const auto& tInputsWithComponentType = get<tComponentType>();

    auto tInputsWithType = std::vector<InputType>{};
    utilities::transform_if(
        tInputsWithComponentType, std::back_inserter(tInputsWithType),
        [](const auto& aInputBlock) { return aInputBlock.mInput.template get<InputType>(); },
        [](const auto& aInputBlock) { return aInputBlock.mInput.template holds_expected_type<InputType>(); });
    return tInputsWithType;
}
}  // namespace plato::input_parser

#endif
