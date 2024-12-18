#include <gtest/gtest.h>

#include "plato/input_parser/documentation_generator/library/LatexFormatter.hpp"

namespace plato::input_parser::documentation_generator::library::unittest
{
namespace
{
const auto kDocOne = Documentation{{"a", "b", "c"}};
const auto kDocTwo = Documentation{{"d", "e", "f"}, {"g", "h", "i"}};
const std::map<std::string, Documentation> kDocumentation = {{"one", kDocOne}, {"two", kDocTwo}};
}  // namespace
TEST(LatexFormatter, FormatLatex)
{
    const auto tDocumentation = kDocumentation;
    const auto tResult = format_command_latex(tDocumentation);
    ASSERT_EQ(tResult.size(), 2U);
    EXPECT_EQ(tResult.at("one").front(), "\\PlatoCommand{one}{a}{b}{c}");
    EXPECT_EQ(tResult.at("two").front(), "\\PlatoCommand{two}{d}{e}{f}");
    EXPECT_EQ(tResult.at("two").back(), "\\PlatoCommand{two}{g}{h}{i}");
}
}  // namespace plato::input_parser::documentation_generator::library::unittest
