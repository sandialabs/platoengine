#ifndef PLATO_INPUT_VALIDATION_VALIDATEDINPUT
#define PLATO_INPUT_VALIDATION_VALIDATEDINPUT

#include <filesystem>

#include "plato/input_parser/ComponentType.hpp"
#include "plato/input_parser/CrossLinkedInput.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/input_validation/ValidatedInputTypeWrapper.hpp"
#include "plato/utilities/Expected.hpp"

namespace plato::input_validation
{
struct ValidateKey;

/// @brief Input data that has passed validation.
///
/// The purpose of this class is to hold the input data, but using the type, express that it has passed all validation
/// functions. It is only constructable via the free function make_validated_input, which applies all registered
/// validation functions.
class ValidatedInput
{
   public:
    ValidatedInput(input_parser::NewParsedInput aInput, const ValidateKey&);

    /// @brief Returns the parsed input blocks corresponding to @a kComponentType
    template <input_parser::ComponentType kComponentType>
    [[nodiscard]] auto get() const;

   private:
    input_parser::NewParsedInput mRawInput;
};

/// @brief Helper function to construct a ValidatedInput object or return a string containing all validation errors.
///
/// This function must be used to construct ValidatedInput, it is the only function allowed to do so.
/// It first applies all validation functions to @a aInput, and returns any validation errors that are encountered.
[[nodiscard]] auto make_validated_input(const input_parser::CrossLinkedInput& aInput)
    -> utilities::Expected<ValidatedInput, std::string>;

/// @brief Helper function to construct a ValidatedInput object or return a string containing all validation errors.
///
/// This overload first cross-links the input, then performs validation. This will return any cross-linking errors as
/// well as validation errors.
[[nodiscard]] auto make_validated_input(const input_parser::NewParsedInput& aInput)
    -> utilities::Expected<ValidatedInput, std::string>;

/// @brief Parse input contained in the string @a aInput.
///
/// This is mainly for testing, prefer to use parse_and_validate_file.
/// Any errors in parsing or validation result in the unexpected type, which is a string containing all error messages
/// that may be printed to the screen.
[[nodiscard]] auto parse_and_validate_string(std::string_view aInput)
    -> utilities::Expected<ValidatedInput, std::string>;

/// @brief Parse input from file @a aInputFile and then validate the input.
///
/// Any errors in parsing or validation result in the unexpected type, which is a string containing all error messages
/// that may be printed to the screen.
[[nodiscard]] auto parse_and_validate_file(const std::filesystem::path& aFileName)
    -> utilities::Expected<ValidatedInput, std::string>;

/// @brief Validated input block for a given component type
template <input_parser::ComponentType kComponentType>
using ValidatedInputDataBlock = ValidatedInputTypeWrapper<input_parser::InputDataBlock, kComponentType>;

/// @brief Provides the validated input type of the component.
///
/// For components (such as geometry) that are unique in an input deck, this is just ValidatedInputDataBlock.
/// For non-unique components (such as objectives) this is a ValidatedInputTypeWrapper-wrapped vector.
template <input_parser::ComponentType kComponentType>
using ValidatedComponentType = decltype(std::declval<ValidatedInput>().get<kComponentType>());

/// @brief Convenience function to unwrap the original input block struct.
/// @tparam InputBlock The original input type.
/// @throw May throw std::bad_any_cast if InputBlock is not the type held by the wrapper.
template <typename InputBlock, input_parser::ComponentType kComponentType>
[[nodiscard]] auto get_input_block(const ValidatedInputDataBlock<kComponentType>& aValidatedInputDataBlock) -> const
    auto&;

template <input_parser::ComponentType kComponentType>
auto ValidatedInput::get() const
{
    using ValidatedTypeWrapperForComponent = ValidatedInputDataBlock<kComponentType>;
    if constexpr (input_parser::kIsUniqueComponent<kComponentType>)
    {
        assert(mRawInput.get<kComponentType>().size() == 1U);
        return ValidatedTypeWrapperForComponent{mRawInput.get<kComponentType>().front()};
    }
    else
    {
        auto tValidatedInputs = std::vector<ValidatedTypeWrapperForComponent>{};
        std::transform(mRawInput.get<kComponentType>().cbegin(), mRawInput.get<kComponentType>().cend(),
                       std::back_inserter(tValidatedInputs),
                       [](const auto& aInputBlock) { return ValidatedTypeWrapperForComponent{aInputBlock}; });
        return ValidatedInputTypeWrapper<std::vector<ValidatedTypeWrapperForComponent>, kComponentType>{
            std::move(tValidatedInputs)};
    }
}

template <typename InputBlock, input_parser::ComponentType kComponentType>
[[nodiscard]] auto get_input_block(const ValidatedInputDataBlock<kComponentType>& aValidatedInputDataBlock) -> const
    auto&
{
    return aValidatedInputDataBlock.rawInput().mInput.template get<InputBlock>();
}

}  // namespace plato::input_validation

#endif
