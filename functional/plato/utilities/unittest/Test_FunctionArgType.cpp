#include <gtest/gtest.h>

#include <tuple>
#include <type_traits>

#include "plato/utilities/FunctionArgType.hpp"

namespace plato::utilities::unittest
{
namespace
{
constexpr auto kArbitraryValue = 42;

struct Bard
{
    auto operator()(int /*unused*/, double /*unused*/, char /*unused*/) -> int { return kArbitraryValue; }
};
struct Paladin
{
    auto operator()(int /*unused*/, double /*unused*/, char /*unused*/) const -> int { return kArbitraryValue; }
};
struct Rogue
{
    auto operator()(int /*unused*/, char /*unused*/) const -> int { return kArbitraryValue; }
};
}  // namespace

TEST(FunctionArgType, Args)
{
    static_assert(std::tuple_size_v<typename FunctionArgType<Bard>::args> == 3U);
    static_assert(std::is_same_v<int, std::tuple_element_t<0U, typename FunctionArgType<Bard>::args>>);
    static_assert(std::is_same_v<double, std::tuple_element_t<1U, typename FunctionArgType<Bard>::args>>);
    static_assert(std::is_same_v<char, std::tuple_element_t<2U, typename FunctionArgType<Bard>::args>>);
}

TEST(FunctionArgType, Arg)
{
    static_assert(std::is_same_v<int, typename FunctionArgType<Paladin>::arg<0U>>);
    static_assert(std::is_same_v<double, typename FunctionArgType<Paladin>::arg<1U>>);
    static_assert(std::is_same_v<char, typename FunctionArgType<Paladin>::arg<2U>>);
}

TEST(FunctionArgType, NumberOfArgs)
{
    static_assert(FunctionArgType<Paladin>::mNumberOfArgs == 3U);
    static_assert(FunctionArgType<Rogue>::mNumberOfArgs == 2U);
}
}  // namespace plato::utilities::unittest
