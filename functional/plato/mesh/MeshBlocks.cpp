#include "plato/mesh/MeshBlocks.hpp"

#include <cassert>

#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"

namespace plato::mesh
{

MeshBlocks::MeshBlocks(Mesh aMeshBase) : Mesh{std::move(aMeshBase)} {}

auto MeshBlocks::blockID(std::string_view /*aBlockName*/) const -> std::optional<MeshBlocks::BlockIDType>
{
    return std::nullopt;
}

auto MeshBlocks::blockData() const -> std::vector<third_party_integration::common::BlockData>
{
    return third_party_integration::stk_io::block_data(bulkData());
}

auto MeshBlocks::nodeIDs(const std::string_view aBlockName) const -> std::vector<std::size_t>
{
    if (const auto tPart = third_party_integration::stk_io::part_with_block_name(bulkData(), aBlockName))
    {
        return third_party_integration::stk_io::node_ids(bulkData(), tPart.value().get());
    }
    else
    {
        return {};
    }
}

auto MeshBlocks::elementIDs(const std::string_view aBlockName) const -> std::vector<std::size_t>
{
    if (const auto tPart = third_party_integration::stk_io::part_with_block_name(bulkData(), aBlockName))
    {
        return third_party_integration::stk_io::element_ids(bulkData(), tPart.value().get());
    }
    else
    {
        return {};
    }
}
}  // namespace plato::mesh
