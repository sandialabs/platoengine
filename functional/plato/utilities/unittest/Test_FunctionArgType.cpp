#include <gtest/gtest.h>

#include "plato/utilities/FunctionArgType.hpp"

namespace plato::utilities::unittest
{
namespace
{
struct Bard
{
    int operator()(int, double, char) { return 42; }
};
struct Paladin
{
    int operator()(int, double, char) const { return 42; }
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
}  // namespace plato::utilities::unittest
