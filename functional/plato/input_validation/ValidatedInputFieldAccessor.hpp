#ifndef PLATO_INPUT_VALIDATION_VALIDATEDINPUTFIELDACCESSOR
#define PLATO_INPUT_VALIDATION_VALIDATEDINPUTFIELDACCESSOR

#include <functional>

#include "plato/input_parser/ComponentBlockParser.hpp"
#include "plato/input_parser/ComponentType.hpp"
#include "plato/input_validation/ValidatedInputTypeWrapper.hpp"
#include "plato/utilities/FunctionArgType.hpp"

namespace plato::input_validation
{
/// @brief A helper for accessing the fields of an input struct wrapped in a ValidatedInputTypeWrapper.
///
/// The main use of this is for accessing CrossReference inputs from factories that require validated input types.
/// We only want to use validated input in factories, but because the input type may not be known by the factory,
/// we need a way to access the CrossReference field within an input.
template <typename FieldType, input_parser::ComponentType kComponentType>
class ValidatedInputFieldAccessor
{
   public:
    using ValidatedInputDataBlock = ValidatedInputTypeWrapper<input_parser::InputDataBlock, kComponentType>;

    template <typename Accessor>
    ValidatedInputFieldAccessor(const Accessor& aAccessor);

    /// @brief Returns the field contained in @a aValidatedInputDataBlock, accessed by the function used to construct
    /// this object.
    ///
    /// @pre The type of the input block held by aValidatedInputDataBlock must be the same as the type used to create
    /// the accessor.
    [[nodiscard]] auto getField(const ValidatedInputDataBlock& aValidatedInputDataBlock) const -> const FieldType&;

   private:
    using AccessorFunction = std::function<const FieldType&(const input_parser::CrossReferencedInput&)>;

    AccessorFunction mAccessor;
};

template <typename FieldType, input_parser::ComponentType kComponentType>
template <typename Accessor>
ValidatedInputFieldAccessor<FieldType, kComponentType>::ValidatedInputFieldAccessor(const Accessor& aAccessor)
    : mAccessor{[aAccessor](const input_parser::CrossReferencedInput& aInput) -> const FieldType&
                {
                    using InputBlockType = typename utilities::FunctionArgType<Accessor>::template arg<0U>;
                    assert(aInput.holds_expected_type<InputBlockType>());
                    return aAccessor(aInput.get<InputBlockType>());
                }}
{
}

template <typename FieldType, input_parser::ComponentType kComponentType>
auto ValidatedInputFieldAccessor<FieldType, kComponentType>::getField(
    const ValidatedInputDataBlock& aValidatedInputDataBlock) const -> const FieldType&
{
    return mAccessor(aValidatedInputDataBlock.rawInput().mInput);
}

}  // namespace plato::input_validation

#endif
