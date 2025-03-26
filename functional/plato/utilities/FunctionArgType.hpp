#ifndef PLATO_UTILITIES_FUNCTIONARGTYPE
#define PLATO_UTILITIES_FUNCTIONARGTYPE

#include <tuple>

namespace plato::utilities
{
namespace detail
{
template <typename F>
struct FunctionArgTypeImpl
{
};
}  // namespace detail

/// Helper for deducing the argument type of a callable object @a F. @a F must have an `operator()` defined.
///
/// For valid types, a tuple containing the call operator's arguments is provided as the member type `args`, and a
/// specific type by the type template `arg`, parameterized by the argument's number.
/// For example, given:
/// @code{.cpp}
/// struct Foo{ int operator()(int, double, char){ return 42; }; };
/// @endcode
/// the following are true:
/// @code{.cpp}
/// std::is_same_v<typename FunctionArgType<Foo>::template arg<0>, int>;
/// std::is_same_v<typename FunctionArgType<Foo>::template arg<1>, double>;
/// std::is_same_v<typename FunctionArgType<Foo>::template arg<2>, char>;
/// @endif
template <typename Function>
struct FunctionArgType
{
    using args = typename detail::FunctionArgTypeImpl<decltype(&Function::operator())>::args;

    template <unsigned int kArg>
    using arg = typename detail::FunctionArgTypeImpl<decltype(&Function::operator())>::template arg<kArg>;

    constexpr static inline std::size_t kNumberOfArgs = std::tuple_size_v<args>;
};

namespace detail
{
template <typename F, typename R, typename... Args>
struct FunctionArgTypeImpl<R (F::*)(Args...)>
{
    using args = std::tuple<Args...>;

    template <unsigned int kArg>
    using arg = std::tuple_element_t<kArg, args>;
};

template <typename F, typename R, typename... Args>
struct FunctionArgTypeImpl<R (F::*)(Args...) const>
{
    using args = std::tuple<Args...>;

    template <unsigned int kArg>
    using arg = std::tuple_element_t<kArg, args>;
};
}  // namespace detail
}  // namespace plato::utilities

#endif
