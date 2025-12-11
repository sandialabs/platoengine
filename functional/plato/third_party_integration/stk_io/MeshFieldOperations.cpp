#include "plato/third_party_integration/stk_io/MeshFieldOperations.hpp"

#include <algorithm>

namespace plato::third_party_integration::stk_io
{
namespace detail
{
auto global_to_local_node_index(const std::vector<std::size_t>& aSortedGlobalNodeIDs, const std::size_t aGlobalNodeID)
    -> std::size_t
{
    assert(std::ranges::is_sorted(aSortedGlobalNodeIDs));

    const auto tLowerBoundIterator = std::ranges::lower_bound(aSortedGlobalNodeIDs, aGlobalNodeID);

    assert(tLowerBoundIterator != aSortedGlobalNodeIDs.end());
    assert(*tLowerBoundIterator == aGlobalNodeID);

    return std::distance(aSortedGlobalNodeIDs.begin(), tLowerBoundIterator);
}
}  // namespace detail
}  // namespace plato::third_party_integration::stk_io
