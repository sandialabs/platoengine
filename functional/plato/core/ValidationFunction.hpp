#ifndef PLATO_CORE_VALIDATIONFUNCTION
#define PLATO_CORE_VALIDATIONFUNCTION

#include <functional>
#include <optional>
#include <string>

#include "plato/input_parser/CrossReference.hpp"
#include "plato/utilities/FunctionArgType.hpp"

namespace plato::core
{
/// @brief An interface for validation functions that helps convert the type-erased input to the expected input
/// struct type.
///
/// This object is to be constructed with a lambda that implements a validation on an input struct. The input struct
/// must be the only argument to the lambda and it is expected to return an optional string. For example:
/// @code{.cpp}
/// const auto tValidationFunction =
///     ValidationFunction{[](const kernel_filter& aInput){return aInput.filter_radius > 0.0;}};
/// @endcode
/// This wraps a validation function checking that the kernel filter radius is positive and non-zero. The validation is
/// called with the validate member function, which is passed a type-erased input type that is converted to the correct
/// type before being passed to the stored function.
class ValidationFunction
{
   public:
    /// @brief Construct from a function object with the signature: `std::optional<std::string>(input_type)`, where
    /// `input_type` is a struct used for parsed input of a component.
    template <typename F>
    ValidationFunction(F aFunction);

    /// @brief Validate @a aInput based on the validation function used to construct this object.
    [[nodiscard]] auto validate(const input_parser::CrossReferencedInput& aInput) const -> std::optional<std::string>;

   private:
    std::function<std::optional<std::string>(const input_parser::CrossReferencedInput& aInput)> mValidationFunction;
};

template <typename F>
ValidationFunction::ValidationFunction(F aFunction)
    : mValidationFunction{[tFunction = std::move(aFunction)](const input_parser::CrossReferencedInput& aInput)
                          {
                              static_assert(utilities::FunctionArgType<F>::kNumberOfArgs == 1U,
                                            "A ValidationFunction must be constructed with a function object having a "
                                            "single argument.");

                              using InputType = typename utilities::FunctionArgType<F>::template arg<0U>;
                              const auto& tCastInput = aInput.get<const InputType&>();
                              return tFunction(tCastInput);
                          }}
{
}

}  // namespace plato::core

#endif
