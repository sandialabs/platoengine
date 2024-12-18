#include <gtest/gtest.h>

#include <sstream>
#include <string_view>
#include <vector>

#include "plato/utilities/FormatText.hpp"
namespace plato::utilities::unittest
{

namespace
{
constexpr auto kTestString = std::string_view{"Extraordinary claims require extraordinary evidence. -Carl Sagan"};
constexpr auto kTestStringTwo = std::string_view{"Now I am become death the destroyer of worlds. -Ken"};

[[nodiscard]] auto tokenize_by_return_line(const std::string& aString) -> std::vector<std::string>
{
    std::vector<std::string> tResult;
    std::istringstream tStream(aString);
    std::string tLine;

    while (std::getline(tStream, tLine))
    {
        tResult.push_back(tLine);
    }

    return tResult;
}

void verify_line_split_correctly(const std::string& aString, const TextWidth aWidth, const TextIndent aIndent)
{
    std::stringstream aStringStream;
    utilities::word_block_justify(aString, aWidth, aIndent, aStringStream);
    const auto tTokenizedString = tokenize_by_return_line(aStringStream.str());
    for (const auto& tLine : tTokenizedString)
    {
        EXPECT_LE(tLine.length(), aWidth.mValue + aIndent.mValue);
        ASSERT_GT(tLine.length(), aIndent.mValue);
        EXPECT_EQ(tLine[aIndent.mValue - 1], ' ');
    }
}

}  // namespace

TEST(FormatText, WordBlockJustify)
{
    verify_line_split_correctly(std::string{kTestString}, TextWidth{55}, TextIndent{10});

    verify_line_split_correctly(std::string{kTestStringTwo}, TextWidth{50}, TextIndent{5});
}

}  // namespace plato::utilities::unittest
