#ifndef PLATO_INPUT_VALIDATION_VALIDATEDINPUT
#define PLATO_INPUT_VALIDATION_VALIDATEDINPUT

#include "plato/input_parser/ComponentType.hpp"
#include "plato/input_parser/CrossLinkedInput.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/input_validation/ValidatedInputTypeWrapper.hpp"
#include "plato/utilities/Expected.hpp"

namespace plato::input_validation
{
class ValidatedInput;

/// @brief Key for implementing the pass-key idiom for ValidatedInput. The key is only constructible by the function
/// make_validated_input.
struct ValidateKey
{
    friend auto make_validated_input(const input_parser::CrossLinkedInput& input)
        -> utilities::Expected<ValidatedInput, std::string>;

   private:
    ValidateKey() {}
    ValidateKey(const ValidateKey&) {}
};

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

template <input_parser::ComponentType kComponentType>
auto ValidatedInput::get() const
{
    if constexpr (input_parser::kIsNamedComponent<kComponentType>)
    {
        auto tValidatedInputs = std::vector<ValidatedInputTypeWrapper<input_parser::InputDataBlock>>{};
        std::transform(mRawInput.get<kComponentType>().cbegin(), mRawInput.get<kComponentType>().cend(),
                       std::back_inserter(tValidatedInputs),
                       [](const auto& aInputBlock) { return ValidatedInputTypeWrapper{aInputBlock}; });
        return ValidatedInputTypeWrapper{std::move(tValidatedInputs)};
    }
    else
    {
        assert(mRawInput.get<kComponentType>().size() == 1U);
        return ValidatedInputTypeWrapper{mRawInput.get<kComponentType>().front()};
    }
}

}  // namespace plato::input_validation

#endif
