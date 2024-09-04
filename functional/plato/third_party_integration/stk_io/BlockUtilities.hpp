#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_BLOCKUTILITIES
#define PLATO_THIRDPARTYINTEGRATION_STKIO_BLOCKUTILITIES

#include <functional>
#include <optional>
#include <stk_mesh/base/Types.hpp>
#include <string>
#include <vector>

#include "plato/third_party_integration/common/BlockData.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"

namespace stk::mesh
{
// In trilinos 15.1, there are conflicting forward declarations of BulkData, of which clang-tidy disapproves.
class BulkData;  // NOLINT
class Part;
}  // namespace stk::mesh

namespace plato::third_party_integration::stk_io
{
using OptionalPartReference = std::optional<std::reference_wrapper<const stk::mesh::Part>>;

/// @brief Given a STK bulk data @a aBulk, return the number of blocks.
[[nodiscard]] unsigned int block_size(const stk::mesh::BulkData& aBulk);

/// @brief Returns all names and ids of the blocks contained in @a aBulk
/// @post The elements of the returned vector are sorted in ascending order based on the block id.
[[nodiscard]] std::vector<common::BlockData> block_data(const stk::mesh::BulkData& aBulk);

/// @brief Returns the Part associated with block with name @a aBlockID if the block exists, `nullopt` otherwise.
[[nodiscard]] auto part_with_block_meta_data_ordinal(const stk::mesh::BulkData& aBulkData,
                                                     common::BlockData::BlockOrdinalType aBlockOrdinal)
    -> OptionalPartReference;

/// @brief Returns the number of elements in the part @a aPart associated with BulkData @a aBulkData.
[[nodiscard]] std::size_t element_size(const stk::mesh::BulkData& aBulkData, const stk::mesh::Part& aPart);

/// @brief Returns the number of elements in the part @a aPart associated with BulkData @a aBulkData.
[[nodiscard]] std::size_t node_size(const stk::mesh::BulkData& aBulkData, const stk::mesh::Part& aPart);

/// @brief Returns the list of node IDs associated with the block @a aPart in mesh @a aBulkData.
[[nodiscard]] std::vector<std::size_t> node_ids(const stk::mesh::BulkData& aBulkData, const stk::mesh::Part& aPart);

/// @brief Returns the list of element IDs associated with the block @a aPart in mesh @a aBulkData.
[[nodiscard]] std::vector<std::size_t> element_ids(const stk::mesh::BulkData& aBulkData, const stk::mesh::Part& aPart);

/// @brief Returns the user-defined element blocks defined in @a aBulkData.
[[nodiscard]] auto element_blocks_parts(const stk::mesh::BulkData& aBulkData) -> stk::mesh::PartVector;

}  // namespace plato::third_party_integration::stk_io

#endif
