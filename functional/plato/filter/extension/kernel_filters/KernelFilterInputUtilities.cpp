#include "plato/filter/extension/kernel_filters/KernelFilterInputUtilities.hpp"

#include <algorithm>
#include <unordered_set>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/filter/extension/CommonInputValidation.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilder.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/FixedBlockUtilities.hpp"

namespace plato::filter::extension::kernel_filters
{

namespace detail
{

auto search_results_contain_all_row_ids(const third_party_integration::stk_search::SearchResults& aSearchResults,
                                        const std::size_t aSize) -> bool
{
    std::unordered_set<third_party_integration::tpetra::TpetraGlobalOrdinal> tSearchResultsSet;
    std::transform(aSearchResults.begin(), aSearchResults.end(),
                   std::inserter(tSearchResultsSet, tSearchResultsSet.end()),
                   [](const auto& aSearchResult) { return aSearchResult.first.id(); });

    std::vector<third_party_integration::tpetra::TpetraGlobalOrdinal> tAllRowIDs(aSize, 0);
    std::iota(tAllRowIDs.begin(), tAllRowIDs.end(), 0);

    return std::all_of(tAllRowIDs.begin(), tAllRowIDs.end(), [&tSearchResultsSet](auto aID)
                       { return tSearchResultsSet.find(aID) != tSearchResultsSet.end(); });
}

}  // namespace detail
}  // namespace plato::filter::extension::kernel_filters
