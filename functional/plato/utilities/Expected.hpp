#ifndef PLATO_UTILITIES_EXPECTED
#define PLATO_UTILITIES_EXPECTED

#include <type_traits>
#include <variant>

#include "plato/utilities/Exception.hpp"

namespace plato::utilities
{
/// @brief A wrapper for an unexpected error type.
///
/// This may be used for assigning an error type to an expected object, though prefer the free function unexpected to
/// deduce the type.
template <typename Error>
struct Unexpected
{
    Error mError;
};

/// @brief Helper function for deducing the error type.
///
/// This may be used for assigning an error type to an expected object. The assignment is converting, so the following
/// will work:
/// @code{.cpp}
/// auto tExpected = Expected<int, std::string>{42};
//  tExpected = unexpected("Error!");
/// @endcode
template <typename Error>
constexpr auto unexpected(Error&& aError) -> Unexpected<Error>;

template <typename T>
constexpr bool kIsUnexpected = false;

template <typename U>
constexpr bool kIsUnexpected<Unexpected<U>> = true;

/// @brief An minimal implementation of std::expected until we can use c++23
///
/// The main use of this is to express an error condition using optional-like semantics. The main differences between
/// this type and `optional` is that this has an error member function that will return an error object. See also
/// https://en.cppreference.com/w/cpp/utility/expected
template <typename T, typename Error>
class Expected
{
    static_assert(!kIsUnexpected<T>, "Expected may not be instantiated with an Unexpected type.");

   public:
    constexpr Expected() = default;

    template <typename U, typename = std::enable_if<std::is_convertible_v<U, T>>>
    constexpr Expected(U&& aU);

    template <typename E, typename = std::enable_if<std::is_convertible_v<E, Error>>>
    constexpr Expected(Unexpected<E>&& aError);

    template <typename E, typename = std::enable_if<std::is_convertible_v<E, Error>>>
    constexpr auto operator=(Unexpected<E>&& aError) -> Expected&;

    /// @brief Returns whether or not an expected object is held (not an error)
    [[nodiscard]] constexpr auto hasValue() const -> bool;

    /// @brief Returns whether or not an expected object is held (not an error)
    [[nodiscard]] constexpr operator bool() const;

    /// @brief If an expected object is held, the value is returned, if not, an exception is thrown with the error if it
    /// is convertible to a string.
    /// @throws Exception
    [[nodiscard]] constexpr auto value() const& -> const T&;

    /// @brief If an expected object is held, the value is returned, if not, an exception is thrown with the error if it
    /// is convertible to a string.
    /// @throws Exception
    [[nodiscard]] constexpr auto value() && -> T&&;

    /// @brief If an expected object is held, the value is returned, if not, an exception is thrown with the error if it
    /// is convertible to a string.
    /// @throws Exception
    [[nodiscard]] constexpr auto value() & -> T&;

    /// @brief Returns the error object if an unexpected type is held (hasValue is `false`)
    /// @pre hasValue returns `false`
    /// @throw May throw `std::bad_variant_access`
    [[nodiscard]] constexpr auto error() const -> const Error&;

   private:
    constexpr void handleBadAccess() const;

    std::variant<T, Unexpected<Error>> mValue;
};

template <typename T, typename Error>
template <typename U, typename>
constexpr Expected<T, Error>::Expected(U&& aU) : mValue{std::forward<U>(aU)}
{
}

template <typename T, typename Error>
template <typename E, typename>
constexpr Expected<T, Error>::Expected(Unexpected<E>&& aError) : mValue{std::forward<Unexpected<E>>(aError)}
{
}

template <typename T, typename Error>
template <typename E, typename>
constexpr auto Expected<T, Error>::operator=(Unexpected<E>&& aError) -> Expected&
{
    mValue = Unexpected<Error>{std::forward<E>(aError.mError)};
    return *this;
}

template <typename T, typename Error>
constexpr auto Expected<T, Error>::hasValue() const -> bool
{
    return std::holds_alternative<T>(mValue);
}

template <typename T, typename Error>
constexpr Expected<T, Error>::operator bool() const
{
    return hasValue();
}

template <typename T, typename Error>
constexpr void Expected<T, Error>::handleBadAccess() const
{
    using DecayedErrorType = std::decay_t<Error>;
    if constexpr (std::is_convertible_v<DecayedErrorType, std::string>)
    {
        throw Exception{std::string{std::get<Unexpected<Error>>(mValue).mError}};
    }
    throw Exception{"Bad expected access."};
}

template <typename T, typename Error>
constexpr auto Expected<T, Error>::value() const& -> const T&
{
    if (hasValue())
    {
        return std::get<T>(mValue);
    }
    handleBadAccess();
    __builtin_unreachable();
}

template <typename T, typename Error>
constexpr auto Expected<T, Error>::value() && -> T&&
{
    if (hasValue())
    {
        return std::get<T>(std::move(mValue));
    }
    handleBadAccess();
    __builtin_unreachable();
}

template <typename T, typename Error>
constexpr auto Expected<T, Error>::value() & -> T&
{
    if (hasValue())
    {
        return std::get<T>(mValue);
    }
    handleBadAccess();
    __builtin_unreachable();
}

template <typename T, typename Error>
constexpr auto Expected<T, Error>::error() const -> const Error&
{
    return std::get<Unexpected<Error>>(mValue).mError;
}

template <typename Error>
constexpr auto unexpected(Error&& aError) -> Unexpected<Error>
{
    return Unexpected<Error>{std::forward<Error>(aError)};
}

}  // namespace plato::utilities

#endif
