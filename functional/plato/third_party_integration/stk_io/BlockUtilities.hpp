#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_BLOCKUTILITIES
#define PLATO_THIRDPARTYINTEGRATION_STKIO_BLOCKUTILITIES

#include <optional>
#include <string>
#include <vector>

#include "plato/third_party_integration/common/BlockData.hpp"

namespace stk::mesh
{
// In trilinos 15.1, there are conflicting forward declarations of BulkData, of which clang-tidy disapproves.
class BulkData;  // NOLINT
}  // namespace stk::mesh

namespace plato::third_party_integration::stk_io
{
/// @brief Given a STK bulk data @a aBulk, return the number of blocks.
[[nodiscard]] unsigned int block_size(const stk::mesh::BulkData& aBulk);

/// @brief Returns all names and ids of the blocks contained in @a aBulk
[[nodiscard]] std::vector<common::BlockData> block_data(const stk::mesh::BulkData& aBulk);

}  // namespace plato::third_party_integration::stk_io

#endif
