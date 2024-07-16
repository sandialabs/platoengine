#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"

#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/Part.hpp>

namespace plato::third_party_integration::stk_io
{
unsigned int block_size(const stk::mesh::BulkData& aBulk) { return aBulk.mesh_meta_data().get_mesh_parts().size(); }

std::vector<common::BlockData> block_data(const stk::mesh::BulkData& aBulk)
{
    const auto& tParts = aBulk.mesh_meta_data().get_mesh_parts();
    auto tBlockData = std::vector<common::BlockData>{};
    tBlockData.reserve(tParts.size());
    std::transform(tParts.cbegin(), tParts.cend(), std::back_inserter(tBlockData),
                   [](const auto& aPart) {
                       return common::BlockData{/*.mID=*/aPart->id(), /*.mName=*/aPart->name()};
                   });

    return tBlockData;
}
}  // namespace plato::third_party_integration::stk_io
