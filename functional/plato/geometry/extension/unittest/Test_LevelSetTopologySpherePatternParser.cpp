#include <gtest/gtest.h>

#include "plato/geometry/extension/LevelSetTopologySpherePatternParser.hpp"
#include "plato/input_parser/test_utilities/TestHelpers.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::geometry::extension::cubit::unittest
{

TEST(SpherePattern, ValidInputParsingTest)
{
    constexpr auto tValidInput = std::string_view{"radius 1.1 spacing 2.2 min (0.1,0.2,0.3) max (10,11,12)"};
    const auto tGold = input_parser::SpherePattern{/*.radius=*/1.1, /*.spacing=*/2.2,
                                                   input_parser::Point{0.1, 0.2, 0.3}, input_parser::Point{10, 11, 12}};
    input_parser::test_utilities::expect_valid_input(tValidInput, tGold, TEST_CONTEXT("Valid input"));
}

TEST(SpherePattern, InvalidInputParsingTest)
{
    {
        constexpr auto tInvalidInput = std::string_view{" spacing 2.2 min (0.1,0.2,0.3) max (10,11,12)"};
        input_parser::test_utilities::expect_invalid_input<input_parser::SpherePattern>(tInvalidInput,
                                                                                        TEST_CONTEXT("Missing radius"));
    }
    {
        constexpr auto tInvalidInput = std::string_view{"radius 1.1  2.2 min (0.1,0.2,0.3) max (10,11,12)"};
        input_parser::test_utilities::expect_invalid_input<input_parser::SpherePattern>(
            tInvalidInput, TEST_CONTEXT("Missing spacing"));
    }
    {
        constexpr auto tInvalidInput = std::string_view{"radius 1.1 spacing 2.2 min (0.1:0.2,0.3) max (10,11,12)"};
        input_parser::test_utilities::expect_invalid_input<input_parser::SpherePattern>(
            tInvalidInput, TEST_CONTEXT("Bad syntax on min"));
    }
    {
        constexpr auto tInvalidInput = std::string_view{"radius 1.1 spacing 2.2 min (0.1,0.2,0.3)  (10,11,12)"};
        input_parser::test_utilities::expect_invalid_input<input_parser::SpherePattern>(
            tInvalidInput, TEST_CONTEXT("Bad syntax on max"));
    }
}

}  // namespace plato::geometry::extension::cubit::unittest
