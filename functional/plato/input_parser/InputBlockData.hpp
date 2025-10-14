#ifndef PLATO_INPUT_PARSER_INPUTBLOCKDATA
#define PLATO_INPUT_PARSER_INPUTBLOCKDATA

#include <any>
#include <string>

#include "plato/components/ComponentType.hpp"

namespace plato::input_parser
{
/// @brief A type-erased wrapper for input structs, which may be of any type.
class InputBlockWrapper
{
   public:
    InputBlockWrapper() = default;

    template <typename T, typename = std::enable_if_t<!std::is_convertible_v<T, InputBlockWrapper>>>
    explicit InputBlockWrapper(T&& aInitialValue);

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
    components::ComponentType mComponentType;
    std::string mBlockName;
    InputBlockWrapper mInput;
};

template <typename T, typename>
InputBlockWrapper::InputBlockWrapper(T&& aInitialValue) : mInput{std::forward<T>(aInitialValue)}
{
}

template <typename T>
auto InputBlockWrapper::get() const -> const T&
{
    assert(holdsExpectedType<T>());
    return std::any_cast<const T&>(mInput);
}

template <typename T>
auto InputBlockWrapper::get() -> T&
{
    assert(holdsExpectedType<T>());
    return std::any_cast<T&>(mInput);
}

template <typename T>
void InputBlockWrapper::set(T&& aValue)
{
    mInput = std::forward<T>(aValue);
}

template <typename T>
auto InputBlockWrapper::holdsExpectedType() const -> bool
{
    return mInput.type() == typeid(T);
}

}  // namespace plato::input_parser

#endif
