#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_SIDESETUTILITIES
#define PLATO_THIRDPARTYINTEGRATION_STKIO_SIDESETUTILITIES

// #include <functional>
// #include <optional>
// #include <stk_mesh/base/Types.hpp>
// #include <string>
#include <vector>

/*
#include "plato/third_party_integration/common/BlockData.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
*/
#include "plato/third_party_integration/stk_io/Triangle.hpp"

namespace stk::mesh
{
// In trilinos 15.1, there are conflicting forward declarations of BulkData, of which clang-tidy disapproves.
class BulkData;  // NOLINT
class Part;
}  // namespace stk::mesh

namespace plato::third_party_integration::stk_io
{

}  // namespace plato::third_party_integration::stk_io

#endif
