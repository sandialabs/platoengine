#include "plato/third_party_integration/stk_io/MeshFieldOperations.hpp"

#include <algorithm>

namespace plato::third_party_integration::stk_io
{
namespace detail
{
auto global_to_local_index(const std::vector<std::size_t>& aSortedGlobalIDs, const std::size_t aGlobalID) -> std::size_t
{
    assert(std::ranges::is_sorted(aSortedGlobalIDs));

    const auto tLowerBoundIterator = std::ranges::lower_bound(aSortedGlobalIDs, aGlobalID);

    assert(tLowerBoundIterator != aSortedGlobalIDs.end());
    assert(*tLowerBoundIterator == aGlobalID);

    return std::distance(aSortedGlobalIDs.begin(), tLowerBoundIterator);
}
}  // namespace detail
}  // namespace plato::third_party_integration::stk_io
