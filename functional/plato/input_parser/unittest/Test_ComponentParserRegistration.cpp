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
// clang-format on

namespace plato::input_parser::unittest
{
namespace
{
[[maybe_unused]] const auto tRegisteredParser =
    ComponentParserRegistration<statically_registered_test_block, ComponentType::kFilter>{};
}

TEST(ComponentParserRegistration, RegisterInline)
{
    [[maybe_unused]] const auto tRegisteredParserInLine =
        ComponentParserRegistration<registration_test_block, ComponentType::kGeometry>{};
    const auto& tRegisteredParsers = registered_component_parsers();
    EXPECT_NE(tRegisteredParsers.find("registration_test_block"), tRegisteredParsers.cend());
}

TEST(ComponentParserRegistration, RegisterStatically)
{
    const auto& tRegisteredParsers = registered_component_parsers();
    EXPECT_NE(tRegisteredParsers.find("statically_registered_test_block"), tRegisteredParsers.cend());
}

}  // namespace plato::input_parser::unittest
