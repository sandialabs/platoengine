#include "plato/utilities/LevenshteinDistance.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <locale>
#include <numeric>
#include <ranges>
#include <string_view>
#include <vector>

namespace plato::utilities::detail
{
namespace
{
[[nodiscard]] auto lowercase_ascii(const char aCharacter) -> char { return std::tolower(aCharacter, std::locale{}); }

}  // namespace

auto levenshtein_distance(std::string_view aLeft, std::string_view aRight) -> std::size_t
{
    if (aLeft.size() > aRight.size())
    {
        std::swap(aLeft, aRight);
    }

    const auto tShortSize = aLeft.size();
    const auto tLongSize = aRight.size();

    auto tPreviousRow = std::vector<std::size_t>(tShortSize + 1);
    auto tCurrentRow = std::vector<std::size_t>(tShortSize + 1);

    std::iota(tPreviousRow.begin(), tPreviousRow.end(), std::size_t{0});

    for (const auto tLongIndex : std::views::iota(std::size_t{1}, tLongSize + 1))
    {
        tCurrentRow[0] = tLongIndex;

        for (const auto tShortIndex : std::views::iota(std::size_t{1}, tShortSize + 1))
        {
            const auto tSubstitutionCost =
                lowercase_ascii(aLeft[tShortIndex - 1]) == lowercase_ascii(aRight[tLongIndex - 1]) ? std::size_t{0}
                                                                                                   : std::size_t{1};

            tCurrentRow[tShortIndex] = std::min({tPreviousRow[tShortIndex] + 1, tCurrentRow[tShortIndex - 1] + 1,
                                                 tPreviousRow[tShortIndex - 1] + tSubstitutionCost});
        }

        std::swap(tPreviousRow, tCurrentRow);
    }

    return tPreviousRow[tShortSize];
}
}  // namespace plato::utilities::detail
