#ifndef PLATO_UTILITIES_VALUEORTAG
#define PLATO_UTILITIES_VALUEORTAG

#include <type_traits>
#include <variant>

namespace plato::utilities
{
/// @brief The purpose of this wrapper is to express a value or some non-value sentinel-like tag.
///
/// This is similar to `std::optional` but can be more descriptive. For example, if we want to get a value from a vector
/// based on a specific index or the last value, this can be expressed as:
/// @code{.cpp}
/// struct LastValue{};
/// using IndexOrLast = ValueOrTag<std::size_t, LastValue>;
/// ...
/// auto get_value(const std::vector<double>& aVector, const IndexOrLast aIndex) {
///   return aVector.at(aIndex.valueOr(aVector.size() - 1));
/// }
/// ...
/// const auto tValue = get_value(tVector, LastValue{});
/// @endcode
///
/// At the call site, the type tag can be used, which may be more expressive than `std::nullopt`.
template <typename Value, typename Tag>
class ValueOrTag
{
   public:
    ValueOrTag(Value aValue);     // Permit implicit conversions
    ValueOrTag(const Tag& aTag);  // Permit implicit conversions

    /// @brief Returns whether or not a value is currently held.
    [[nodiscard]] auto hasValue() const -> bool;

    /// @brief Returns whether or not no value is held.
    template <typename T, typename = std::enable_if_t<std::is_same_v<T, Tag>>>
    [[nodiscard]] auto has() const -> bool;

    /// @brief Returns a copy of the current value if a value is held, or @a aTagValue.
    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U, Value>>>
    [[nodiscard]] auto valueOr(U&& aTagValue) const& -> Value;

    /// @brief Moves the current value if a value is held, or returns @a aTagValue.
    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U, Value>>>
    [[nodiscard]] auto valueOr(U&& aTagValue) && -> Value;

    /// @brief Returns the current value, or the result of invoking @a aFunction.
    ///
    /// The function will only be invoked if no value is held.
    template <typename F, typename = std::enable_if_t<std::is_convertible_v<std::invoke_result_t<F>, Value>>>
    [[nodiscard]] auto valueOrInvoke(const F& aFunction) const& -> Value;

    /// @brief Moves the current value, or the result of invoking @a aFunction.
    ///
    /// The function will only be invoked if no value is held.
    template <typename F, typename = std::enable_if_t<std::is_convertible_v<std::invoke_result_t<F>, Value>>>
    [[nodiscard]] auto valueOrInvoke(const F& aFunction) && -> Value;

   private:
    std::variant<Value, Tag> mValue;
};

template <typename Value, typename Tag>
ValueOrTag<Value, Tag>::ValueOrTag(Value aValue) : mValue{std::move(aValue)}
{
}

template <typename Value, typename Tag>
ValueOrTag<Value, Tag>::ValueOrTag(const Tag& aTag) : mValue{std::move(aTag)}
{
}

template <typename Value, typename Tag>
auto ValueOrTag<Value, Tag>::hasValue() const -> bool
{
    return std::holds_alternative<Value>(mValue);
}

template <typename Value, typename Tag>
template <typename T, typename>
auto ValueOrTag<Value, Tag>::has() const -> bool
{
    return std::holds_alternative<Tag>(mValue);
}

template <typename Value, typename Tag>
template <typename U, typename>
auto ValueOrTag<Value, Tag>::valueOr(U&& aTagValue) const& -> Value
{
    return valueOrInvoke([&aTagValue]() { return std::forward<U>(aTagValue); });
}

template <typename Value, typename Tag>
template <typename U, typename>
auto ValueOrTag<Value, Tag>::valueOr(U&& aTagValue) && -> Value
{
    return std::move(*this).valueOrInvoke([&aTagValue]() { return std::forward<U>(aTagValue); });
}

template <typename Value, typename Tag>
template <typename F, typename>
auto ValueOrTag<Value, Tag>::valueOrInvoke(const F& aFunction) const& -> Value
{
    if (hasValue())
    {
        return std::get<Value>(mValue);
    }
    return aFunction();
}

template <typename Value, typename Tag>
template <typename F, typename>
auto ValueOrTag<Value, Tag>::valueOrInvoke(const F& aFunction) && -> Value
{
    if (hasValue())
    {
        return std::get<Value>(std::move(mValue));
    }
    return aFunction();
}
}  // namespace plato::utilities

#endif
