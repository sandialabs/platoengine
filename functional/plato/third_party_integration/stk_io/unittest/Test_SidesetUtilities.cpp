#include <gtest/gtest.h>

#include <stk_io/FillMesh.hpp>
#include <stk_io/StkMeshIoBroker.hpp>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/SidesetUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::third_party_integration::stk_io::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMeshWithNodeSets;
using third_party_integration::stk_io::test_utilities::TwoBlockMeshOnDisk;
using third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;

constexpr auto kExpectedNumberOfElementsInBlock1 = 273u;
constexpr auto kExpectedNumberOfElementsInBlock2 = 40u;
constexpr auto kExpectedNumberOfNodesInBlock1 = 93u;
constexpr auto kExpectedNumberOfNodesInBlock2 = 90u;

}  // namespace

}  // namespace plato::third_party_integration::stk_io::unittest
