#include <gtest/gtest.h>

#include <vector>

#include "plato/criteria/library/CriterionDocumentation.hpp"
#include "plato/main/documentation_generator/library/Utilities.hpp"

namespace plato::main::documentation_generator::library::unittest
{
TEST(Utilities, MystTableHeader)
{
    const auto tColumnWidths = {13, 42, 45};
    const auto tColumnLabels = {"Han", "Chewie", "Leia"};
    const auto tTableHeader = myst_table_header(tColumnWidths, tColumnLabels);

    constexpr auto tExpected =
        std::string_view{"```{list-table}\n:header-rows: 1\n:widths: 13 42 45\n\n* - Han\n  - Chewie\n  - Leia\n\n"};

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

TEST(Utilities, ConsolidatedCriteriaDocumentation)
{
    {
        auto tOriginalDocumentation = std::vector<criteria::library::CriterionDocumentation>{
            {.mAppName = "apple", .mCriterionName = "macbook", .mTraits = {{"type", "laptop"}}},
            {.mAppName = "dell", .mCriterionName = "xps", .mTraits = {{"type", "laptop"}}},
            {.mAppName = "ibm", .mCriterionName = "thinkpad", .mTraits = {{"type", "laptop"}}},
            {.mAppName = "dell", .mCriterionName = "xps", .mTraits = {{"type", "desktop"}}}};

        const auto tConsolidatedDocumentation = consolidated_criteria_documentation(tOriginalDocumentation);

        const auto tExpected = std::vector<criteria::library::CriterionDocumentation>{
            {.mAppName = "apple", .mCriterionName = "macbook", .mTraits = {{"type", "laptop"}}},
            {.mAppName = "dell", .mCriterionName = "xps", .mTraits = {{"type", "laptop, desktop"}}},
            {.mAppName = "ibm", .mCriterionName = "thinkpad", .mTraits = {{"type", "laptop"}}}};
        EXPECT_EQ(tConsolidatedDocumentation, tExpected) << "Single trait";
    }
    {
        auto tOriginalDocumentation = std::vector<criteria::library::CriterionDocumentation>{
            {.mAppName = "apple", .mCriterionName = "macbook", .mTraits = {{"type", "laptop"}, {"cpu", "apple"}}},
            {.mAppName = "dell", .mCriterionName = "xps", .mTraits = {{"type", "laptop"}, {"cpu", "intel"}}},
            {.mAppName = "dell", .mCriterionName = "xps", .mTraits = {{"type", "desktop"}, {"cpu", "intel"}}}};

        const auto tConsolidatedDocumentation = consolidated_criteria_documentation(tOriginalDocumentation);

        const auto tExpected = std::vector<criteria::library::CriterionDocumentation>{
            {.mAppName = "apple", .mCriterionName = "macbook", .mTraits = {{"type", "laptop"}, {"cpu", "apple"}}},
            {.mAppName = "dell", .mCriterionName = "xps", .mTraits = {{"type", "laptop, desktop"}, {"cpu", "intel"}}}};
        EXPECT_EQ(tConsolidatedDocumentation, tExpected) << "Multiple traits";
    }
}

}  // namespace plato::main::documentation_generator::library::unittest
