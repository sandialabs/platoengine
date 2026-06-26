#ifndef PLATO_MAIN_DOCUMENTATION_GENERATOR_LIBRARY_UTILITIES
#define PLATO_MAIN_DOCUMENTATION_GENERATOR_LIBRARY_UTILITIES

#include <numeric>
#include <ranges>
#include <string>

namespace plato::main::documentation_generator::library
{
/// @brief Generates the header of a Myst-parser-style markdown table.
/// @pre Neither @a aColumnWidths nor @a aColumnLabels may be empty.
template <std::ranges::range ColumnWidths, std::ranges::range ColumnLabels>
    requires(std::is_convertible_v<std::ranges::range_value_t<ColumnWidths>, int> &&
             std::is_convertible_v<std::ranges::range_value_t<ColumnLabels>, std::string>)
[[nodiscard]] auto myst_table_header(const ColumnWidths& aColumnWidths,
                                     const ColumnLabels& aColumnLabels) -> std::string;

/// @brief Generates a row of a Myst-parser-style markdown table.
/// @pre @a aColumnEntries may not be empty.
template <std::ranges::range ColumnLabels>
    requires(std::is_convertible_v<std::ranges::range_value_t<ColumnLabels>, std::string>)
[[nodiscard]] auto myst_table_row(const ColumnLabels& aColumnEntries) -> std::string;

/// @brief Returns the ending for a Myst-parser table, which is just ```
[[nodiscard]] auto myst_table_footer() -> std::string;

template <std::ranges::range ColumnWidths, std::ranges::range ColumnLabels>
    requires(std::is_convertible_v<std::ranges::range_value_t<ColumnWidths>, int> &&
             std::is_convertible_v<std::ranges::range_value_t<ColumnLabels>, std::string>)
auto myst_table_header(const ColumnWidths& aColumnWidths, const ColumnLabels& aColumnLabels) -> std::string
{
    assert(std::ranges::begin(aColumnWidths) != std::ranges::end(aColumnWidths));
    assert(std::ranges::begin(aColumnLabels) != std::ranges::end(aColumnLabels));

    constexpr auto tPreamble = std::string_view{"```{list-table}\n:header-rows: 1\n:widths:"};
    auto tWidths = std::accumulate(std::ranges::begin(aColumnWidths), std::ranges::end(aColumnWidths), std::string{},
                                   [](std::string&& aCurrent, const auto aWidth)
                                   { return std::move(aCurrent) + std::string{" "} + std::to_string(aWidth); });

    auto tResult = std::string{tPreamble};
    tResult += std::move(tWidths);
    tResult += std::string{"\n\n"};
    tResult += myst_table_row(aColumnLabels);
    return tResult;
}

template <std::ranges::range ColumnLabels>
    requires(std::is_convertible_v<std::ranges::range_value_t<ColumnLabels>, std::string>)
auto myst_table_row(const ColumnLabels& aColumnEntries) -> std::string
{
    const auto tColumnLabelsBegin = std::ranges::begin(aColumnEntries);
    auto tFirstEntry = std::string{"* - "} + *tColumnLabelsBegin + std::string{"\n"};
    auto tEntries =
        std::accumulate(std::next(tColumnLabelsBegin), std::ranges::end(aColumnEntries), std::move(tFirstEntry),
                        [](std::string&& aCurrent, std::string aLabel)
                        { return std::move(aCurrent) + std::string{"  - "} + std::move(aLabel) + std::string{"\n"}; });
    return std::move(tEntries) + std::string{"\n"};
}

}  // namespace plato::main::documentation_generator::library

#endif
