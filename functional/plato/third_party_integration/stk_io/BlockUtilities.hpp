#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_BLOCKUTILITIES
#define PLATO_THIRDPARTYINTEGRATION_STKIO_BLOCKUTILITIES

#include <functional>
#include <optional>
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
/// @brief Given a STK bulk data @a aBulk, return the number of blocks.
[[nodiscard]] unsigned int block_size(const stk::mesh::BulkData& aBulk);

/// @brief Returns all names and ids of the blocks contained in @a aBulk
/// @post The elements of the returned vector are sorted in ascending order based on the block id.
[[nodiscard]] std::vector<common::BlockData> block_data(const stk::mesh::BulkData& aBulk);

/// @brief Returns the Part associated with block with name @a aBlockName if the block exists, `nullptr` otherwise.
[[nodiscard]] auto part_with_block_name(const stk::mesh::BulkData& aBulkData, std::string_view aBlockName)
    -> std::optional<std::reference_wrapper<stk::mesh::Part>>;

/// @brief Returns the number of elements in the part @a aPart associated with BulkData @a aBulkData.
[[nodiscard]] std::size_t element_size(const stk::mesh::BulkData& aBulkData, const stk::mesh::Part& aPart);

/// @brief Returns the number of elements in the part @a aPart associated with BulkData @a aBulkData.
[[nodiscard]] std::size_t node_size(const stk::mesh::BulkData& aBulkData, const stk::mesh::Part& aPart);

/// @brief Returns the list of node IDs associated with the block @a aPart in mesh @a aBulkData.
[[nodiscard]] std::vector<std::size_t> node_ids(const stk::mesh::BulkData& aBulkData, const stk::mesh::Part& aPart);

/// @brief Returns the list of element IDs associated with the block @a aPart in mesh @a aBulkData.
[[nodiscard]] std::vector<std::size_t> element_ids(const stk::mesh::BulkData& aBulkData, const stk::mesh::Part& aPart);

}  // namespace plato::third_party_integration::stk_io

#endif
