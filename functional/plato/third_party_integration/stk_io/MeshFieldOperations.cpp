#include "plato/third_party_integration/stk_io/MeshFieldOperations.hpp"

#include <bits/ranges_algo.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/GetEntities.hpp>
#include <stk_mesh/base/Types.hpp>
#include <stk_topology/topology_decl.hpp>
#include <vector>

#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"

namespace plato::third_party_integration::stk_io::detail
{
auto global_to_local_index(const std::vector<std::size_t>& aSortedGlobalIDs, const std::size_t aGlobalID) -> std::size_t
{
    assert(std::ranges::is_sorted(aSortedGlobalIDs));

    const auto tLowerBoundIterator = std::ranges::lower_bound(aSortedGlobalIDs, aGlobalID);

    assert(tLowerBoundIterator != aSortedGlobalIDs.end());
    assert(*tLowerBoundIterator == aGlobalID);

    return std::distance(aSortedGlobalIDs.begin(), tLowerBoundIterator);
}
auto element_entities(const stk::mesh::BulkData& aBulkData, const PartReferenceVector& aParts)
    -> stk::mesh::EntityVector
{
    auto tEntities = stk::mesh::EntityVector{};
    constexpr auto tSortedByGlobalID = true;
    stk::mesh::get_entities(aBulkData, stk::topology::ELEMENT_RANK, detail::parts_to_selector(aParts), tEntities,
                            tSortedByGlobalID);
    return tEntities;
}

}  // namespace plato::third_party_integration::stk_io::detail
