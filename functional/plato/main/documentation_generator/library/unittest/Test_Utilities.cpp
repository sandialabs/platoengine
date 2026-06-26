#include <gtest/gtest.h>

#include "plato/main/documentation_generator/library/Utilities.hpp"

namespace plato::main::documentation_generator::library::unittest
{
TEST(Utilities, MystTableHeader)
{
    const auto tColumnWidths = {13, 42, 45};
    const auto tColumnLabels = {"Han", "Chewie", "Leia"};
    const auto tTableHeader = myst_table_header(tColumnWidths, tColumnLabels);

    constexpr auto tExpected =
        std::string_view{"```{{list-table}}\n:header-rows: 1\n:widths: 13 42 45\n\n* - Han\n  - Chewie\n  - Leia\n\n"};

    EXPECT_EQ(tExpected, tTableHeader);
}

TEST(Utilities, MystTableRow)
{
    const auto tColumnEntries = {"Han", "Chewie"};
    const auto tTableHeader = myst_table_row(tColumnEntries);

    constexpr auto tExpected = std::string_view{"* - Han\n  - Chewie\n\n"};

    EXPECT_EQ(tExpected, tTableHeader);
}

TEST(Utilities, MystTableFooter)
{
    constexpr auto tExpected = std::string_view{"```"};

    EXPECT_EQ(tExpected, myst_table_footer());
}

}  // namespace plato::main::documentation_generator::library::unittest
