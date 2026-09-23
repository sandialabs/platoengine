#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"

#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <cassert>
#include <set>
#include <stk_mesh/base/Types.hpp>
#include <string>
#include <utility>
#include <vector>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/krino/BackgroundMesh.hpp"
#include "plato/third_party_integration/krino/BackgroundMeshUtilities.hpp"
#include "plato/third_party_integration/krino/CutMeshUtilities.hpp"
#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"
#include "plato/third_party_integration/krino/SensitivityMapUtilities.hpp"
#include "plato/third_party_integration/krino/SnappingParameters.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/utilities/DataFilePath.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::third_party_integration::krino::test_utilities
{

namespace
{

const auto kThreeQuarterOffsetXHatPlane = Plane{{-1, 0, 0}, 0.75};
const auto kFourTriTwoBlockMeshFilePath = utilities::data_file_path("four_tri_two_block.cdf");
}  // namespace

void test_sensitivity_map(const SensitivityMap& aResult,
                          const SensitivityMap& aGold,
                          const plato::test_utilities::TestContext& aTestContext)
{
    ASSERT_EQ(aResult.size(), aGold.size()) << aTestContext;
    for (const auto& tEntry : aResult)
    {
        const auto& tGoldEntry = aGold.at(tEntry.first);
        ASSERT_EQ(tGoldEntry.mBackgroundMeshNodeIDs, tEntry.second.mBackgroundMeshNodeIDs);

        for (const auto [tGold, tResult] :
             utilities::Zip(tGoldEntry.mNodalSensitivities, tEntry.second.mNodalSensitivities))
        {
            common::test_utilities::test_double_equality_of_components(tResult, tGold, aTestContext);
        }
        EXPECT_EQ(tGoldEntry.mDesignDomainLocalIndex, tEntry.second.mDesignDomainLocalIndex) << aTestContext;
    }
}

void four_tri_test_on_background_node_ids(const plato::test_utilities::TestContext& aTestContext)
{
    ASSERT_TRUE(kFourTriTwoBlockMeshFilePath.has_value());
    const auto tBackgroundMesh = background_mesh_from_primitives(
        kFourTriTwoBlockMeshFilePath.value(), LevelSetPrimitives{{kThreeQuarterOffsetXHatPlane}, {}});
    cut_mesh(tBackgroundMesh, SnappingParameters{});

    const auto tResult = background_node_ids(tBackgroundMesh);
    const auto tGold = std::vector<stk::mesh::EntityId>{1, 2, 4, 5, 6, 7};
    EXPECT_EQ(tGold, tResult) << aTestContext;
}

auto background_mesh_from_vector(const std::filesystem::path& aFileName,
                                 const std::vector<double>& aVector,
                                 const std::set<std::string>& aExcludedBlocks) -> BackgroundMesh
{
    auto tBackgroundMesh = detail::initialize_background_mesh(aFileName, aExcludedBlocks);
    assert(boost::mpi::communicator{}.size() == 1);
    const auto tNodes = node_entities_in_mesh(tBackgroundMesh);
    assert(tNodes.size() == aVector.size());
    for (const auto [tNode, tValue] : utilities::Zip(tNodes, aVector))
    {
        level_set_value(tBackgroundMesh.levelSetFields(), tNode) = tValue;
    }
    return tBackgroundMesh;
}

}  // namespace plato::third_party_integration::krino::test_utilities
