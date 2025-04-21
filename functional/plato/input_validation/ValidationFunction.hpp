#ifndef PLATO_INPUT_VALIDATION_VALIDATIONFUNCTION
#define PLATO_INPUT_VALIDATION_VALIDATIONFUNCTION

#include <functional>
#include <optional>
#include <string>

#include "plato/input_parser/CrossReference.hpp"
#include "plato/utilities/FunctionArgType.hpp"

namespace plato::input_validation
{
/// @brief An interface for validation functions that can help convert the type-erased input to the expected input
/// struct type.
///
/// This object is to be constructed with a lambda that implements a validation on an input struct or other type. The
/// validation function must return an optional string. For example:
/// @code{.cpp}
/// const auto tValidationFunction =
///     ValidationFunction{[](const kernel_filter& aInput){return aInput.filter_radius > 0.0;}};
/// @endcode
/// This wraps a validation function checking that the kernel filter radius is positive and non-zero. The validation is
/// called with the validate member function, which is passed a type-erased input type that is converted to the correct
/// type before being passed to the stored function.
template <typename InputToValidate, typename... AdditionalArgs>
class ValidationFunction
{
   public:
    /// @brief Construct from a function object with the signature: `std::optional<std::string>(input_type)`, where
    /// `input_type` is a struct used for parsed input of a component.
    template <typename F>
    ValidationFunction(F aFunction);

    /// @brief Validate @a aInput based on the validation function used to construct this object.
    [[nodiscard]] auto validate(const InputToValidate& aInput, AdditionalArgs... aAdditionalArgs) const
        -> std::optional<std::string>;

   private:
    std::function<std::optional<std::string>(const InputToValidate&, AdditionalArgs...)> mValidationFunction;
};

template <typename... AdditionalArgs>
using InputBlockWrapperValidationFunction = ValidationFunction<input_parser::InputBlockWrapper, AdditionalArgs...>;

namespace detail
{
template <typename F, typename... AdditionalArgs>
auto validate_input_block(const F aValidationFunction,
                          const input_parser::InputBlockWrapper& aInput,
                          AdditionalArgs... aAdditionalArgs) -> std::optional<std::string>
{
    using InputType = typename utilities::FunctionArgType<F>::template arg<0U>;
    if (aInput.template holdsExpectedType<InputType>())
    {
        return aValidationFunction(aInput.template get<const InputType&>(), std::move(aAdditionalArgs)...);
    }
    return std::nullopt;
}

}  // namespace detail

template <typename InputToValidate, typename... AdditionalArgs>
template <typename F>
ValidationFunction<InputToValidate, AdditionalArgs...>::ValidationFunction(F aFunction)
    : mValidationFunction{
          [tFunction = std::move(aFunction)](const InputToValidate& aInput,
                                             AdditionalArgs... aAdditionalArgs) -> std::optional<std::string>
          {
              if constexpr (std::is_same_v<InputToValidate, input_parser::InputBlockWrapper>)
              {
                  return detail::validate_input_block(tFunction, aInput, std::move(aAdditionalArgs)...);
              }
              else
              {
                  return tFunction(aInput, std::move(aAdditionalArgs)...);
              }
          }}
{
}

template <typename InputToValidate, typename... AdditionalArgs>
auto ValidationFunction<InputToValidate, AdditionalArgs...>::validate(const InputToValidate& aInput,
                                                                      AdditionalArgs... aAdditionalArgs) const
    -> std::optional<std::string>
{
    return mValidationFunction(aInput, std::move(aAdditionalArgs)...);
}

}  // namespace plato::input_validation

#endif
