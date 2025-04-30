#include <gtest/gtest.h>

#include "plato/input_parser/ComponentBlockParser.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"

// clang-format off
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         registration_test_block,
                         plato::input_parser::ComponentType::kGeometry,
                         (int, dark_roast, "no comment")
                         (bool, medium_roast, "no comment")
                         (unsigned, light_roast, "no comment"))

PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         statically_registered_test_block,
                         plato::input_parser::ComponentType::kFilter,
                         (int, dark_roast, "no comment"))

PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         another_filter,
                         plato::input_parser::ComponentType::kFilter,
                         (int, dark_roast, "no comment"))

// clang-format on

namespace plato::input_parser::unittest
{
namespace
{
[[maybe_unused]] const auto tRegisteredParser = ComponentParserRegistration<statically_registered_test_block>{};
}

TEST(ComponentParserRegistration, RegisterInline)
{
    [[maybe_unused]] const auto tRegisteredParserInLine = ComponentParserRegistration<registration_test_block>{};
    const auto& tRegisteredParsers = registered_component_parsers();
    EXPECT_NE(tRegisteredParsers.find("registration_test_block"), tRegisteredParsers.cend());
}

TEST(ComponentParserRegistration, RegisterStatically)
{
    const auto& tRegisteredParsers = registered_component_parsers();
    EXPECT_NE(tRegisteredParsers.find("statically_registered_test_block"), tRegisteredParsers.cend());
}

TEST(ComponentParserRegistration, BlockNamesOfComponentType)
{
    [[maybe_unused]] const auto tRegisteredParserInLine = ComponentParserRegistration<registration_test_block>{};

    {
        const auto tGeometryNames = all_block_names_with_component_type<ComponentType::kGeometry>();
        const auto tExpectedGeometryNames = std::vector<std::string>{"registration_test_block"};
        EXPECT_EQ(tGeometryNames, tExpectedGeometryNames);
    }
    {
        const auto tRegisteredParsers = registered_component_parsers();  // Save existing parsers to restore later
        [[maybe_unused]] const auto tAnotherRegisteredFilter = ComponentParserRegistration<another_filter>{};
        const auto tFilterNames = all_block_names_with_component_type<ComponentType::kFilter>();
        const auto tExpectedFilterNames =
            std::vector<std::string>{"another_filter", "statically_registered_test_block"};
        EXPECT_EQ(tFilterNames, tExpectedFilterNames);
        registered_component_parsers() = tRegisteredParsers;
    }
}

}  // namespace plato::input_parser::unittest
