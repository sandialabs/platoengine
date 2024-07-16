#ifndef PLATO_THIRDPARTYINTEGRATION_COMMON_BLOCKDATA
#define PLATO_THIRDPARTYINTEGRATION_COMMON_BLOCKDATA

#include <string>

namespace plato::third_party_integration::common
{
/// @brief Struct for holding a block's name and ID, as read from an exodus mesh
struct BlockData
{
    int64_t mID = 0;
    std::string mName{};
};

}  // namespace plato::third_party_integration::common

#endif
