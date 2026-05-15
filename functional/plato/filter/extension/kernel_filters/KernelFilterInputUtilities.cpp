#include "plato/filter/extension/kernel_filters/KernelFilterInputUtilities.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <numeric>
#include <ranges>
#include <set>
#include <unordered_set>
#include <vector>

#include "plato/third_party_integration/stk_search/Utilities.hpp"
#include "plato/third_party_integration/tpetra/Utilities.hpp"

namespace plato::filter::extension::kernel_filters::detail
{

auto search_result_ids(const third_party_integration::stk_search::SearchResults& aSearchResults) -> std::vector<int>
{
    const auto tIDView = aSearchResults | std::ranges::views::transform([](const auto& aSearchResult)
                                                                        { return aSearchResult.first.id(); });
    const auto tUniqueIds = std::set<int>(tIDView.begin(), tIDView.end());
    return {tUniqueIds.begin(), tUniqueIds.end()};
}

auto search_results_contain_all_row_ids(const std::vector<int>& aSearchResults, const std::size_t aSize) -> bool
{
    std::unordered_set<int> tSearchResultsSet;
    std::transform(aSearchResults.begin(), aSearchResults.end(),
                   std::inserter(tSearchResultsSet, tSearchResultsSet.end()),
                   [](const auto& aSearchResult) { return aSearchResult; });

    std::vector<third_party_integration::tpetra::TpetraGlobalOrdinal> tAllRowIDs(aSize, 0);
    std::iota(tAllRowIDs.begin(), tAllRowIDs.end(), 0);

    return std::all_of(tAllRowIDs.begin(), tAllRowIDs.end(), [&tSearchResultsSet](auto aID)
                       { return tSearchResultsSet.find(aID) != tSearchResultsSet.end(); });
}

}  // namespace plato::filter::extension::kernel_filters::detail
