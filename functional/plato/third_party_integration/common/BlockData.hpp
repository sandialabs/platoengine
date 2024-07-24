#ifndef PLATO_THIRDPARTYINTEGRATION_COMMON_BLOCKDATA
#define PLATO_THIRDPARTYINTEGRATION_COMMON_BLOCKDATA

#include <string>

namespace plato::third_party_integration::common
{
/// @brief Struct for holding a block's name and IDs, as read from an exodus mesh
struct BlockData
{
    using BlockIDType = int64_t;
    using BlockOrdinalType = unsigned;

    BlockIDType mID = 0;
    BlockOrdinalType mMetaDataOrdinal = 0u;  // An internally assigned stk ID
    std::string mName{};
};

}  // namespace plato::third_party_integration::common

#endif
