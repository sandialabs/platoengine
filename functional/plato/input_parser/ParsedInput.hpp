#ifndef PLATO_INPUT_PARSER_PARSEDINPUT
#define PLATO_INPUT_PARSER_PARSEDINPUT

#include <array>
#include <unordered_map>
#include <vector>

#include "plato/components/ComponentType.hpp"
#include "plato/input_parser/BlockStructRule.hpp"
#include "plato/input_parser/InputBlockData.hpp"
#include "plato/utilities/EnumIndexing.hpp"
#include "plato/utilities/Expected.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::input_parser
{
class ComponentBlockParser;
}

namespace plato::input_parser
{
/// @brief The in-memory representation of a parsed input deck.
///
/// This class contains the result of a parsed input deck. The input is organized into vectors that are retrievable with
/// the component type.
/// @note The input stored in this class has not been validated and so is not guaranteed to have the correct number of
/// components for any component type.
class ParsedInput
{
   public:
    ParsedInput() = default;

    /// @brief @a aRawInput is the result of parsing an input deck without organizing the results by component type.
    ParsedInput(std::vector<InputDataBlock> aRawInput);

    /// @brief Returns the parsed input blocks corresponding to @a kComponentType
    template <components::ComponentType kComponentType>
    [[nodiscard]] auto get() const -> const std::vector<InputDataBlock>&;

    /// @brief Returns the parsed input blocks corresponding to @a kComponentType
    template <components::ComponentType kComponentType>
    [[nodiscard]] auto get() -> std::vector<InputDataBlock>&;

    /// @brief Returns the parsed input blocks corresponding to the type @a InputType
    ///
    /// This may be used to retrieve a specific input block.
    template <typename InputType>
    [[nodiscard]] auto get() const -> std::vector<InputType>;

   private:
    static constexpr inline std::size_t kNumberOfComponents =
        utilities::number_of_enumerates<components::ComponentType>();
    std::array<std::vector<InputDataBlock>, kNumberOfComponents> mInputBlocks;
};

/// @brief Parse the string @a aInput to a ParsedInput object using the registered parsers.
[[nodiscard]] auto make_parsed_input(const std::string& aInput) -> utilities::Expected<ParsedInput, std::string>;

/// @brief Parse the string @a aInput to a ParsedInput object.
[[nodiscard]] auto make_parsed_input(const std::string& aInput,
                                     const std::unordered_map<std::string, ComponentBlockParser>& aComponentParsers)
    -> utilities::Expected<ParsedInput, std::string>;

template <components::ComponentType kComponentType>
auto ParsedInput::get() -> std::vector<InputDataBlock>&
{
    static_assert(kComponentType != components::ComponentType::kNumberOfEnumerates,
                  "ParsedInput::get must only be instantiated with a valid components::ComponentType.");
    return std::get<utilities::enum_index(kComponentType)>(mInputBlocks);
}

template <components::ComponentType kComponentType>
auto ParsedInput::get() const -> const std::vector<InputDataBlock>&
{
    static_assert(kComponentType != components::ComponentType::kNumberOfEnumerates,
                  "ParsedInput::get must only be instantiated with a valid components::ComponentType.");
    return std::get<utilities::enum_index(kComponentType)>(mInputBlocks);
}

template <typename InputType>
auto ParsedInput::get() const -> std::vector<InputType>
{
    constexpr auto tComponentType = ComponentTypeOfInputBlock<InputType>::value;
    const auto& tInputsWithComponentType = get<tComponentType>();

    auto tInputsWithType = std::vector<InputType>{};
    utilities::transform_if(
        tInputsWithComponentType, std::back_inserter(tInputsWithType),
        [](const auto& aInputBlock) { return aInputBlock.mInput.template get<InputType>(); },
        [](const auto& aInputBlock) { return aInputBlock.mInput.template holdsExpectedType<InputType>(); });
    return tInputsWithType;
}
}  // namespace plato::input_parser

#endif
