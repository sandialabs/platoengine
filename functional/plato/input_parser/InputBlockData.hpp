#ifndef PLATO_INPUT_PARSER_INPUTBLOCKDATA
#define PLATO_INPUT_PARSER_INPUTBLOCKDATA

#include <any>
#include <string>

#include "plato/input_parser/ComponentType.hpp"

namespace plato::input_parser
{
/// @brief A type-erased wrapper for input structs, which may be of any type.
class CrossReferencedInput
{
   public:
    CrossReferencedInput() = default;

    template <typename T, typename = std::enable_if<!std::is_convertible_v<T, CrossReferencedInput>>>
    explicit CrossReferencedInput(T&& aInitialValue);

    /// @brief Returns the held object
    /// @pre holdsExpectedType must return `true` for type @a T.
    template <typename T>
    [[nodiscard]] auto get() const -> const T&;

    /// @brief Returns the held object
    /// @pre holdsExpectedType must return `true` for type @a T.
    template <typename T>
    [[nodiscard]] auto get() -> T&;

    /// @brief Assigns a new value with @a aValue.
    template <typename T>
    void set(T&& aValue);

    /// @brief Checks that the held object has type @a T.
    template <typename T>
    [[nodiscard]] auto holdsExpectedType() const -> bool;

    /// @brief Checks that the held object has a value.
    [[nodiscard]] auto hasValue() const -> bool;

   private:
    std::any mInput;
};

/// @brief Type-erased wrapper for holding the parsed data of a component input block.
struct InputDataBlock
{
    ComponentType mComponentType;
    std::string mBlockName;
    CrossReferencedInput mInput;
};

template <typename T, typename>
CrossReferencedInput::CrossReferencedInput(T&& aInitialValue) : mInput{std::forward<T>(aInitialValue)}
{
}

template <typename T>
auto CrossReferencedInput::get() const -> const T&
{
    assert(holdsExpectedType<T>());
    return std::any_cast<const T&>(mInput);
}

template <typename T>
auto CrossReferencedInput::get() -> T&
{
    assert(holdsExpectedType<T>());
    return std::any_cast<T&>(mInput);
}

template <typename T>
void CrossReferencedInput::set(T&& aValue)
{
    mInput = std::forward<T>(aValue);
}

template <typename T>
auto CrossReferencedInput::holdsExpectedType() const -> bool
{
    return mInput.type() == typeid(T);
}

}  // namespace plato::input_parser

#endif
