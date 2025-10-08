#include "plato/third_party_integration/stk_io/SidesetUtilities.hpp"

#include <stk_mesh/base/Bucket.hpp>
#include <stk_mesh/base/BulkData.hpp>
// #include <stk_mesh/base/Comm.hpp>
#include <stk_mesh/base/ExodusTranslator.hpp>
// #include <stk_mesh/base/GetEntities.hpp>
// #include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/Part.hpp>
// #include <stk_mesh/base/Selector.hpp>
#include <Akri_OrientedSideNodes.hpp>

namespace plato::third_party_integration::stk_io
{
using PartReferenceVector = std::vector<std::reference_wrapper<const stk::mesh::Part>>;

namespace
{

}  // namespace

}  // namespace plato::third_party_integration::stk_io
