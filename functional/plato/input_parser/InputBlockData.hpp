#ifndef PLATO_INPUT_PARSER_INPUTBLOCKDATA
#define PLATO_INPUT_PARSER_INPUTBLOCKDATA

#include <any>
#include <boost/optional.hpp>
#include <concepts>
#include <functional>
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

    /// @brief Returns true if the object held by this wrapper represents an active input.
    ///
    /// An input is active if: It has no public member named `active` or it has a public member `active` that is a
    /// `boost::optional<bool>` type, and is either empty or `true`.
    [[nodiscard]] auto active() const -> bool;

   private:
    std::any mInput;
    std::function<bool(const std::any&)> mActive;
};

namespace detail
{
template <typename InputType>
constexpr inline bool kHasActiveMember = requires(InputType aInput) {
    { aInput.active } -> std::convertible_to<boost::optional<bool>>;
};

template <typename InputType>
[[nodiscard]] auto active(const std::any& aInput) -> bool
{
    if constexpr (kHasActiveMember<InputType>)
    {
        return std::any_cast<const InputType&>(aInput).active.value_or(true);
    }
    return true;
}
}  // namespace detail

/// @brief Type-erased wrapper for holding the parsed data of a component input block.
struct InputDataBlock
{
    components::ComponentType mComponentType;
    std::string mBlockName;
    InputBlockWrapper mInput;
};

template <typename T, typename>
InputBlockWrapper::InputBlockWrapper(T&& aInitialValue)
    : mInput{std::forward<T>(aInitialValue)}, mActive{[](const std::any& aInput) { return detail::active<T>(aInput); }}
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
