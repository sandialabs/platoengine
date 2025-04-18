#ifndef PLATO_INPUT_VALIDATION_VALIDATEDINPUTTYPEWRAPPER
#define PLATO_INPUT_VALIDATION_VALIDATEDINPUTTYPEWRAPPER

#include <utility>

#include "plato/input_parser/ComponentBlockParser.hpp"
#include "plato/input_parser/ComponentType.hpp"
#include "plato/utilities/FunctionArgType.hpp"

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
template <typename InputType, input_parser::ComponentType kComponentType>
class ValidatedInputTypeWrapper
{
   public:
    using RawInputType = InputType;
    constexpr static inline input_parser::ComponentType mComponentType = kComponentType;

    [[nodiscard]] auto rawInput() const -> const InputType&;

   private:
    template <input_parser::ComponentType kCrossReferenceComponentType,
              input_parser::ComponentType kParentComponentType,
              typename Accessor>
    friend auto validated_cross_reference(
        const ValidatedInputTypeWrapper<input_parser::InputDataBlock, kParentComponentType>& aValidatedInput,
        const Accessor& aAccessor)
        -> ValidatedInputTypeWrapper<input_parser::InputDataBlock, kCrossReferenceComponentType>;

    friend class plato::input_validation::ValidatedInput;

    ValidatedInputTypeWrapper(InputType aRawInput);

    InputType mRawInput;
};

template <typename T>
constexpr inline bool kIsValidatedInputTypeWrapper = false;

template <typename T, input_parser::ComponentType kComponentType>
constexpr inline bool kIsValidatedInputTypeWrapper<ValidatedInputTypeWrapper<T, kComponentType>> = true;

/// @brief Returns a cross reference, accessed using @a aAccessor, wrapped in ValidatedInputTypeWrapper.
///
/// This must be used to access cross-referenced fields that need to be wrapped in ValidatedInputTypeWrapper.
template <input_parser::ComponentType kCrossReferenceComponentType,
          input_parser::ComponentType kComponentType,
          typename Accessor>
[[nodiscard]] auto validated_cross_reference(
    const ValidatedInputTypeWrapper<input_parser::InputDataBlock, kComponentType>& aValidatedInput,
    const Accessor& aAccessor) -> ValidatedInputTypeWrapper<input_parser::InputDataBlock, kCrossReferenceComponentType>;

template <typename InputType, input_parser::ComponentType kComponentType>
ValidatedInputTypeWrapper<InputType, kComponentType>::ValidatedInputTypeWrapper(InputType aRawInput)
    : mRawInput(std::move(aRawInput))
{
}

template <typename InputType, input_parser::ComponentType kComponentType>
auto ValidatedInputTypeWrapper<InputType, kComponentType>::rawInput() const -> const InputType&
{
    return mRawInput;
}

template <input_parser::ComponentType kCrossReferenceComponentType,
          input_parser::ComponentType kComponentType,
          typename Accessor>
[[nodiscard]] auto validated_cross_reference(
    const ValidatedInputTypeWrapper<input_parser::InputDataBlock, kComponentType>& aValidatedInput,
    const Accessor& aAccessor) -> ValidatedInputTypeWrapper<input_parser::InputDataBlock, kCrossReferenceComponentType>
{
    using InputBlockType = typename utilities::FunctionArgType<Accessor>::template arg<0U>;

    assert(aValidatedInput.mRawInput.mInput.template holdsExpectedType<InputBlockType>());
    const auto& tCrossReferenceField = aAccessor(aValidatedInput.mRawInput.mInput.template get<InputBlockType>());

    assert(tCrossReferenceField.has_value());
    return ValidatedInputTypeWrapper<input_parser::InputDataBlock, kCrossReferenceComponentType>{
        input_parser::InputDataBlock{kCrossReferenceComponentType, tCrossReferenceField.value().mName,
                                     tCrossReferenceField.value().mInputBlock}};
}

}  // namespace plato::input_validation

#endif
