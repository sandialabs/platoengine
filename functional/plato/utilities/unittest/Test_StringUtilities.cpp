#include <gtest/gtest.h>

#include <set>
#include <string>
#include <utility>
#include <vector>

#include "plato/utilities/StringUtilities.hpp"

namespace plato::utilities::unittest
{
TEST(Utilities, ConcatenateVector)
{
    const auto tStrings = std::vector<std::string>{"thing", "one", "thing", "two"};
    const auto tResult = concatenate_container(tStrings);
    EXPECT_EQ(tResult, "thing one thing two");
}

TEST(Utilities, ConcatenateVectorCustomDelimiter)
{
    const auto tStrings = std::vector<std::string>{"a", "b", "c"};
    const auto tResult = concatenate_container(tStrings, ", ");
    EXPECT_EQ(tResult, "a, b, c");
}
TEST(Utilities, OneElementVector)
{
    const auto tStrings = std::vector<std::string>{"onestring"};
    const auto tResult = concatenate_container(tStrings);
    EXPECT_EQ(tResult, tStrings.front());
}

TEST(Utilities, EmptyVector)
{
    const auto tStrings = std::vector<std::string>{};
    const auto tResult = concatenate_container(tStrings);
    EXPECT_TRUE(tResult.empty());
}

TEST(Utilities, ConcatenateSet)
{
    const auto tStrings = std::set<std::string>{"f", "e", "d"};
    const auto tResult = concatenate_container(tStrings);
    EXPECT_EQ(tResult, "d e f");
}

TEST(Utilities, ConcatenateStringLValues)
{
    const auto tString1 = std::string{"one"};
    const auto tString2 = std::string{"fish"};
    const auto tResult = concatenate(tString1, tString2);
    EXPECT_EQ(tResult, "onefish");
}

TEST(Utilities, ConcatenateStringRValues)
{
    auto tString1 = std::string{"two"};
    const auto tResult = concatenate(std::move(tString1), std::string{"fish"});
    EXPECT_EQ(tResult, "twofish");
}

TEST(Utilities, ConcatenateStringCharStar)
{
    const auto* const tString1 = "red";
    const auto tResult = concatenate(tString1, "fish");
    EXPECT_EQ(tResult, "redfish");
}

TEST(Utilities, ConcatenateStringView)
{
    const auto tString1 = std::string_view{"blue"};
    const auto tResult = concatenate(tString1, std::string_view{"fish"});
    EXPECT_EQ(tResult, "bluefish");
}

TEST(Utilities, ConcatenateStringsAndNumbers)
{
    const auto tOne = 1;
    const auto* const tFish = "fish";
    const auto tWo = unsigned{2};
    const auto tResult = concatenate(tOne, tFish, tWo, tFish);
    EXPECT_EQ(tResult, "1fish2fish");
}

TEST(Utilities, ConcatenateStringsChar)
{
    const auto tFLetter = 'f';
    const auto tILetter = 'i';
    const auto tResult = concatenate(tFLetter, tILetter, 's', 'h');
    EXPECT_EQ(tResult, "fish");
}

}  // namespace plato::utilities::unittest