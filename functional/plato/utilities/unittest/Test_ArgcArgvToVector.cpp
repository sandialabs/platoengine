#include <gtest/gtest.h>

#include <array>

#include "plato/utilities/ArgcArgvToVector.hpp"

namespace plato::utilities::unittest
{

namespace
{

constexpr auto kArgvJustPlato = std::array{"plato"};
constexpr int kArgcJustPlato = 1;

constexpr auto kArgvPlatoWithInput = std::array{"plato", "input.i"};
constexpr int kArgcPlatoWithInput = 2;

}  // namespace

TEST(ArgcArgvHandler, ArgcArgvToStdVector)
{
    {
        const auto tResult = argc_argv_to_std_vector(kArgcJustPlato, kArgvJustPlato.data());
        ASSERT_EQ(tResult.size(), 0U);
    }
    {
        const auto tResult = argc_argv_to_std_vector(kArgcPlatoWithInput, kArgvPlatoWithInput.data());
        ASSERT_EQ(tResult.size(), 1U);
        EXPECT_EQ(tResult.front(), "input.i");
    }
}

}  // namespace plato::utilities::unittest
