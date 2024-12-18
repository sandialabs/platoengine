#include <gtest/gtest.h>

#include "plato/utilities/ArgcArgvToVector.hpp"

namespace plato::utilities::unittest
{

namespace
{

const char* kArgvJustPlato[] = {"plato"};
constexpr int kArgcJustPlato = 1;

const char* kArgvPlatoWithInput[] = {"plato", "input.i"};
constexpr int kArgcPlatoWithInput = 2;

}  // namespace

TEST(ArgcArgvHandler, ArgcArgvToStdVector)
{
    {
        const auto tResult = argc_argv_to_std_vector(kArgcJustPlato, const_cast<char**>(kArgvJustPlato));
        ASSERT_EQ(tResult.size(), 0U);
    }
    {
        const auto tResult = argc_argv_to_std_vector(kArgcPlatoWithInput, const_cast<char**>(kArgvPlatoWithInput));
        ASSERT_EQ(tResult.size(), 1U);
        EXPECT_EQ(tResult.front(), "input.i");
    }
}

}  // namespace plato::utilities::unittest
