#include "plato/filter/extension/kernel_filters/KernelFilterInputUtilities.hpp"

#include <algorithm>
#include <unordered_set>

namespace plato::filter::extension::kernel_filters
{

namespace detail
{

auto search_result_ids(const third_party_integration::stk_search::SearchResults& aSearchResults) -> std::vector<int>
{
    const auto tIDView = aSearchResults | std::ranges::views::transform([](const auto& aSearchResult)
                                                                        { return aSearchResult.first.id(); });
    const auto tUniqueIds = std::set<int>(tIDView.begin(), tIDView.end());
    return std::vector<int>(tUniqueIds.begin(), tUniqueIds.end());
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

}  // namespace detail
}  // namespace plato::filter::extension::kernel_filters
