#ifndef PLATO_INPUT_VALIDATION_VALIDATEDINPUTTYPEWRAPPER
#define PLATO_INPUT_VALIDATION_VALIDATEDINPUTTYPEWRAPPER

#include <utility>

namespace plato::input_validation
{
class ValidatedInput;
}

namespace plato::input_validation
{
/// @brief A wrapper for a input data that may only be constructed by ValidatedInput using the pass-key idiom.
///
/// The purpose of the wrapper is to ensure, via the type, that input passed to a component has passed validation.
/// After the input is wrapped in this class, only const access is allowed via the rawInput member.
template <typename InputType>
class ValidatedInputTypeWrapper
{
   public:
    using RawInputType = InputType;

    [[nodiscard]] auto rawInput() const -> const InputType&;

   private:
    friend class plato::input_validation::ValidatedInput;

    ValidatedInputTypeWrapper(InputType aRawInput);

    InputType mRawInput;
};

template <typename InputType>
ValidatedInputTypeWrapper<InputType>::ValidatedInputTypeWrapper(InputType aRawInput) : mRawInput(std::move(aRawInput))
{
}

template <typename InputType>
auto ValidatedInputTypeWrapper<InputType>::rawInput() const -> const InputType&
{
    return mRawInput;
}

}  // namespace plato::input_validation

#endif
