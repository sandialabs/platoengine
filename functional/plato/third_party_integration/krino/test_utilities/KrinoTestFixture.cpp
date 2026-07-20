#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"

#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <cassert>
#include <stk_mesh/base/Types.hpp>
#include <utility>
#include <vector>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/krino/BackgroundMesh.hpp"
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
    const auto tBackgroundMesh =
        populate_level_set_field_from_primitives(LevelSetPrimitives{{kThreeQuarterOffsetXHatPlane}, {}},
                                                 initialize_background_mesh(kFourTriTwoBlockMeshFilePath.value()));
    cut_mesh(tBackgroundMesh, SnappingParameters{});

    const auto tResult = background_node_ids(tBackgroundMesh);
    const auto tGold = std::vector<stk::mesh::EntityId>{1, 2, 4, 5, 6, 7};
    EXPECT_EQ(tGold, tResult) << aTestContext;
}

auto populate_level_set_field_from_vector(BackgroundMesh&& aBackgroundMesh,
                                          const std::vector<double>& aVector) -> BackgroundMesh
{
    assert(boost::mpi::communicator{}.size() == 1);
    const auto tNodes = node_entities_in_mesh(aBackgroundMesh);
    assert(tNodes.size() == aVector.size());
    for (const auto [tNode, tValue] : utilities::Zip(tNodes, aVector))
    {
        level_set_value(aBackgroundMesh.mLevelSetFields, tNode) = tValue;
    }
    return std::move(aBackgroundMesh);
}

}  // namespace plato::third_party_integration::krino::test_utilities
