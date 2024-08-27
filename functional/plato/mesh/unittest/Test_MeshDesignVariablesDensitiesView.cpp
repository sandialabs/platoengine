#include <gtest/gtest.h>

#include <boost/core/ignore_unused.hpp>
#include <numeric>

#include "plato/mesh/MeshDesignVariables.hpp"
#include "plato/mesh/MeshDesignVariablesDensitiesView.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/NamedType.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::mesh
{
bool operator==(const Density& aLHS, const Density& aRHS)
{
    return aLHS.mDensity == aRHS.mDensity && aLHS.mGlobalMeshEntityID == aRHS.mGlobalMeshEntityID &&
           aLHS.mDesignVariableVectorIndex == aRHS.mDesignVariableVectorIndex;
}
}  // namespace plato::mesh

namespace plato::mesh::unittest
{
namespace
{
const auto kDensities1 = std::vector{1.0, 0.0, 4.0};
const auto kIDs1 = std::vector<std::size_t>{0, 1, 2};
const auto kBlockDensityVector1 = MeshDesignVariables::DensityVector{
    {kIDs1[0], kIDs1[0], kDensities1[0]}, {kIDs1[1], kIDs1[1], kDensities1[1]}, {kIDs1[2], kIDs1[2], kDensities1[2]}};

const auto kDensities2 = std::vector{-1.0, 10.0};
const auto kIDs2 = std::vector<std::size_t>{0, 1};
const auto kBlockDensityVector2WithOverlap =
    MeshDesignVariables::DensityVector{{kIDs2[0], kIDs2[0], kDensities2[0]}, {kIDs2[1], kIDs2[1], kDensities2[1]}};

const auto kDensities3 = std::vector{-2.0, 11.0};
const auto kIDs3 = std::vector<std::size_t>{3, 4};
const auto kBlockDensityVector2NoOverlap =
    MeshDesignVariables::DensityVector{{kIDs3[0], kIDs3[0], kDensities3[0]}, {kIDs3[1], kIDs3[1], kDensities3[1]}};

const auto kSingleBlockDensities = MeshDesignVariables::BlockDensities{{0, kBlockDensityVector1}};
const auto kTwoBlockDensitiesOverlap =
    MeshDesignVariables::BlockDensities{{0, kBlockDensityVector1}, {1, kBlockDensityVector2WithOverlap}};

const auto kTwoBlockDensitiesNoOverlap =
    MeshDesignVariables::BlockDensities{{0, kBlockDensityVector1}, {1, kBlockDensityVector2NoOverlap}};

using NewDensities = utilities::NamedType<std::vector<Density>, struct NewDensitiesTag>;
using Block1Densities = utilities::NamedType<std::vector<Density>, struct NewDensitiesTag>;
using Block2Densities = utilities::NamedType<std::vector<Density>, struct NewDensitiesTag>;

void check_mutable_view(MeshDesignVariables& aMeshDesignVariables,
                        const NewDensities& aNewDensities,
                        const Block1Densities& aExpectedDensitiesBlock1,
                        const Block2Densities& aExpectedDensitiesBlock2,
                        const plato::test_utilities::TestContext& aTestContext)
{
    const auto tMeshView = MeshDesignVariablesDensitiesMutableView{aMeshDesignVariables};
    std::copy(aNewDensities.mValue.cbegin(), aNewDensities.mValue.cend(), tMeshView.begin());
    EXPECT_EQ(aExpectedDensitiesBlock1.mValue, aMeshDesignVariables.mBlockDensities.at(0)) << aTestContext;
    EXPECT_EQ(aExpectedDensitiesBlock2.mValue, aMeshDesignVariables.mBlockDensities.at(1)) << aTestContext;
}
}  // namespace

TEST(MeshDesignVariablesViews, SizeSingleBlock)
{
    const auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"mars.exo", /*.mBlockDensities=*/kSingleBlockDensities};
    ASSERT_EQ(MeshDesignVariablesDensitiesView{tMeshDesignVariables}.size(), kDensities1.size());
}

TEST(MeshDesignVariablesViews, SizeTwoBlockOverlap)
{
    const auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"ceres.exo", /*.mBlockDensities=*/kTwoBlockDensitiesOverlap};
    ASSERT_EQ(MeshDesignVariablesDensitiesView{tMeshDesignVariables}.size(), kDensities1.size());
}

TEST(MeshDesignVariablesViews, SizeTwoBlockNoOverlap)
{
    const auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"io.exo", /*.mBlockDensities=*/kTwoBlockDensitiesNoOverlap};
    ASSERT_EQ(MeshDesignVariablesDensitiesView{tMeshDesignVariables}.size(), kDensities1.size() + kDensities2.size());
}

TEST(MeshDesignVariablesViews, BeginEnd)
{
    const auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"jupiter.exo", /*.mBlockDensities=*/kSingleBlockDensities};
    const auto tMeshView = MeshDesignVariablesDensitiesView{tMeshDesignVariables};

    const auto tBeginIterator = tMeshView.begin();
    constexpr auto tExpectedIteratorVectorSize = 1U;
    ASSERT_EQ(tBeginIterator.mCurrentIterators.size(), tExpectedIteratorVectorSize);
    EXPECT_EQ(tBeginIterator.mCurrentIterators.front(), tMeshDesignVariables.mBlockDensities.cbegin()->second.cbegin());
    ASSERT_EQ(tBeginIterator.mEndIterators.size(), tExpectedIteratorVectorSize);
    EXPECT_EQ(tBeginIterator.mEndIterators.front(), tMeshDesignVariables.mBlockDensities.cbegin()->second.cend());

    const auto tEndIterator = tMeshView.end();
    ASSERT_EQ(tEndIterator.mCurrentIterators.size(), tExpectedIteratorVectorSize);
    EXPECT_EQ(tEndIterator.mCurrentIterators.front(), tMeshDesignVariables.mBlockDensities.cbegin()->second.cend());
    ASSERT_EQ(tEndIterator.mEndIterators.size(), tExpectedIteratorVectorSize);
    EXPECT_EQ(tEndIterator.mEndIterators.front(), tMeshDesignVariables.mBlockDensities.cbegin()->second.cend());

    // Increment begin until it's at the end and check that they're equal
    auto tIncrementedIterator = tMeshView.begin();
    for (const auto tDensity : kDensities1)
    {
        boost::ignore_unused(tDensity);
        ++tIncrementedIterator;
    }
    EXPECT_EQ(tIncrementedIterator, tEndIterator);
}

TEST(MeshDesignVariablesViews, RangeBasedFor)
{
    const auto tCheckDensities = [](const MeshDesignVariablesDensitiesView aMeshView,
                                    const std::vector<Density>& aExpectedDensities,
                                    const test_utilities::TestContext& aTestContext)
    {
        ASSERT_EQ(aExpectedDensities.size(), aMeshView.size());
        auto tExpectedDensitiesIterator = aExpectedDensities.cbegin();
        for (const Density tDensityValue : aMeshView)
        {
            EXPECT_EQ(tDensityValue.mDensity, tExpectedDensitiesIterator->mDensity) << aTestContext;
            EXPECT_EQ(tDensityValue.mGlobalMeshEntityID, tExpectedDensitiesIterator->mGlobalMeshEntityID)
                << aTestContext;
            ++tExpectedDensitiesIterator;
        }
    };

    // No overlap
    {
        const auto tMeshDesignVariables =
            MeshDesignVariables{/*.mFileName=*/"pluto.exo", /*.mBlockDensities=*/kTwoBlockDensitiesNoOverlap};
        const auto tMeshView = MeshDesignVariablesDensitiesView{tMeshDesignVariables};

        auto tExpectedDensities = kBlockDensityVector1;
        std::copy(kBlockDensityVector2NoOverlap.cbegin(), kBlockDensityVector2NoOverlap.cend(),
                  std::back_inserter(tExpectedDensities));
        tCheckDensities(tMeshView, tExpectedDensities, TEST_CONTEXT("No overlap"));
    }
    // Overlap
    {
        const auto tMeshDesignVariables =
            MeshDesignVariables{/*.mFileName=*/"deimos.exo", /*.mBlockDensities=*/kTwoBlockDensitiesOverlap};
        const auto tMeshView = MeshDesignVariablesDensitiesView{tMeshDesignVariables};

        auto tExpectedDensities = kBlockDensityVector1;
        tCheckDensities(tMeshView, tExpectedDensities, TEST_CONTEXT("Full overlap"));
    }
    // One block
    {
        const auto tMeshDesignVariables =
            MeshDesignVariables{/*.mFileName=*/"venus.exo", /*.mBlockDensities=*/kSingleBlockDensities};
        const auto tMeshView = MeshDesignVariablesDensitiesView{tMeshDesignVariables};

        auto tExpectedDensities = kBlockDensityVector1;
        tCheckDensities(tMeshView, tExpectedDensities, TEST_CONTEXT("Full overlap"));
    }
}

TEST(MeshDesignVariablesViews, MutableView)
{
    auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"uranus.exo", /*.mBlockDensities=*/kSingleBlockDensities};

    static_assert(!kIsConstIterator<std::vector<double>::iterator>);
    static_assert(kIsConstIterator<std::vector<double>::const_iterator>);

    const auto tMeshView = MeshDesignVariablesDensitiesMutableView{tMeshDesignVariables};

    const auto tNewDensities = std::vector{Density{0, 0, 0.0}, Density{1, 1, 1.0}, Density{2, 2, 2.0}};
    std::copy(tNewDensities.cbegin(), tNewDensities.cend(), tMeshView.begin());

    const auto tVectorFromView =
        mesh_design_variables_to_vector(MeshDesignVariablesDensitiesView{tMeshDesignVariables});
    EXPECT_EQ(tVectorFromView, tNewDensities);
}

TEST(MeshDesignVariablesViews, MutableViewTwoBlocksNoOverlap)
{
    auto tMeshDesignVariables = MeshDesignVariables{/*.mFileName*/ "mercury.exo", kTwoBlockDensitiesNoOverlap};
    const auto tNewDensities = std::vector<Density>{{0, 0, 0.0}, {1, 1, 1.0}, {2, 2, 2.0}, {3, 3, 3.0}, {4, 4, 4.0}};
    const auto tExpectedDensitiesBlock1 = std::vector<Density>{{0, 0, 0.0}, {1, 1, 1.0}, {2, 2, 2.0}};
    const auto tExpectedDensitiesBlock2 = std::vector<Density>{{3, 3, 3.0}, {4, 4, 4.0}};
    check_mutable_view(tMeshDesignVariables, NewDensities{tNewDensities}, Block1Densities{tExpectedDensitiesBlock1},
                       Block2Densities{tExpectedDensitiesBlock2}, TEST_CONTEXT("Blocks do not overlap"));
}

TEST(MeshDesignVariablesViews, MutableViewTwoBlocksWithOverlap)
{
    auto tMeshDesignVariables = MeshDesignVariables{/*.mFileName*/ "earth.exo", kTwoBlockDensitiesOverlap};
    const auto tNewDensities = std::vector<Density>{{0, 0, 0.0}, {1, 1, 1.0}, {2, 2, 2.0}};
    const auto tExpectedDensitiesBlock1 = std::vector<Density>{{0, 0, 0.0}, {1, 1, 1.0}, {2, 2, 2.0}};
    const auto tExpectedDensitiesBlock2 = std::vector<Density>{{0, 0, 0.0}, {1, 1, 1.0}};
    check_mutable_view(tMeshDesignVariables, NewDensities{tNewDensities}, Block1Densities{tExpectedDensitiesBlock1},
                       Block2Densities{tExpectedDensitiesBlock2}, TEST_CONTEXT("Blocks overlap"));
}

TEST(MeshDesignVariablesViews, ToVector)
{
    const auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"phobos.exo", /*.mBlockDensities=*/kSingleBlockDensities};
    const auto tVectorFromView =
        mesh_design_variables_to_vector(MeshDesignVariablesDensitiesView{tMeshDesignVariables});
    EXPECT_EQ(tVectorFromView, kBlockDensityVector1);
}

TEST(MeshDesignVariablesViews, ToVectorTwoBlocksOverlap)
{
    const auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"saturn.exo", /*.mBlockDensities=*/kTwoBlockDensitiesOverlap};
    const auto tVectorFromView =
        mesh_design_variables_to_vector(MeshDesignVariablesDensitiesView{tMeshDesignVariables});
    EXPECT_EQ(tVectorFromView, kBlockDensityVector1);
}

TEST(MeshDesignVariablesViews, CombineDensities)
{
    const auto tDensities = detail::combine_densities_and_ids(kDensities1, kIDs1);
    for (const auto& [tResult, tExpected] : utilities::Zip{tDensities, kBlockDensityVector1})
    {
        EXPECT_EQ(tResult.mDensity, tExpected.mDensity);
        EXPECT_EQ(tResult.mGlobalMeshEntityID, tExpected.mGlobalMeshEntityID);
    }
}

TEST(MeshDesignVariablesViews, SplitDensities)
{
    const auto [tDensityValues, tIDMap] = detail::split_densities(kBlockDensityVector1);

    EXPECT_EQ(tDensityValues, kDensities1);
    EXPECT_EQ(tIDMap, kIDs1);
}
}  // namespace plato::mesh::unittest
