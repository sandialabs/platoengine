#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"

#include <mpi.h>

#include <boost/mpi/communicator.hpp>

#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/krino/BackgroundMesh.hpp"
#include "plato/third_party_integration/krino/SnappingParameters.hpp"
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
    const auto tBackgroundMesh = initialize_background_mesh(kFourTriTwoBlockMeshFilePath.value());
    populate_level_set_field_from_primitives(LevelSetPrimitives{{kThreeQuarterOffsetXHatPlane}, {}}, tBackgroundMesh);
    cut_mesh(tBackgroundMesh.mKrinoMesh->bulk_data(), tBackgroundMesh.mLevelSetFields, SnappingParameters{});

    const auto tResult = background_node_ids(*tBackgroundMesh.mKrinoMesh, tBackgroundMesh.mLevelSetFields);
    const auto tGold = std::vector<stk::mesh::EntityId>{1, 2, 4, 5, 6, 7};
    EXPECT_EQ(tGold, tResult) << aTestContext;
}

void populate_level_set_field_from_vector(const BackgroundMesh& aBackgroundMesh, const std::vector<double>& aVector)
{
    assert(boost::mpi::communicator{}.size() == 1);
    // std::vector<::krino::LS_Field> tField = ::krino::Phase_Support::get_levelset_fields(aKrinoMesh.meta_data()); //
    // using this is BAD!
    const auto tNodes = node_entities_in_mesh(*aBackgroundMesh.mKrinoMesh, aBackgroundMesh.mLevelSetFields);
    assert(tNodes.size() == aVector.size());
    for (const auto [tNode, tValue] : utilities::Zip(tNodes, aVector))
    {
        level_set_value(aBackgroundMesh.mLevelSetFields, tNode) = tValue;
    }
}

}  // namespace plato::third_party_integration::krino::test_utilities
