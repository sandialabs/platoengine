#include <gtest/gtest.h>

#include <boost/core/ignore_unused.hpp>
#include <numeric>

#include "plato/mesh/MeshDesignVariables.hpp"
#include "plato/mesh/MeshDesignVariablesViews.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/IndexRange.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::mesh
{
bool operator==(const Density& aLHS, const Density& aRHS)
{
    return aLHS.mDensity == aRHS.mDensity && aLHS.mGlobalID == aRHS.mGlobalID;
}
}  // namespace plato::mesh

namespace plato::mesh::unittest
{
namespace
{
const auto kDensities1 = std::vector{1.0, 0.0, 4.0};
const auto kIDs1 = std::vector<std::size_t>{0, 1, 2};
const auto kBlockDensityVector1 = MeshDesignVariables::DensityVector{
    {kIDs1[0], kDensities1[0]}, {kIDs1[1], kDensities1[1]}, {kIDs1[2], kDensities1[2]}};

const auto kDensities2 = std::vector{-1.0, 10.0};
const auto kBlockDensityVector2 = MeshDesignVariables::DensityVector{{0, kDensities2[0]}, {1, kDensities2[1]}};

const auto kSingleBlockDensities = MeshDesignVariables::BlockDensities{{0, kBlockDensityVector1}};
const auto kTwoBlockDensities =
    MeshDesignVariables::BlockDensities{{0, kBlockDensityVector1}, {1, kBlockDensityVector2}};

using MeshDesignVariablesDensitiesViewConstIterator = MeshDesignVariablesDensitiesView::IteratorType;

}  // namespace

TEST(MeshDesignVariablesViews, IncrementIterator)
{
    auto tIterator = MeshDesignVariablesDensitiesViewConstIterator{
        kSingleBlockDensities.cbegin(), kSingleBlockDensities.cend(), kBlockDensityVector1.cbegin()};
    for (const auto tIndex : utilities::IndexRange{kDensities1.size()})
    {
        boost::ignore_unused(tIndex);
        const auto tIteratorBeforeIncrement = tIterator;
        ++tIterator;
        const auto tDistance = std::distance(tIteratorBeforeIncrement, tIterator);
        EXPECT_EQ(tDistance, 1u);
    }
}

TEST(MeshDesignVariablesViews, DereferenceIterator)
{
    auto tIterator = MeshDesignVariablesDensitiesViewConstIterator{
        kSingleBlockDensities.cbegin(), kSingleBlockDensities.cend(), kBlockDensityVector1.cbegin()};
    EXPECT_EQ((*tIterator).mDensity, kBlockDensityVector1.cbegin()->mDensity);
    EXPECT_EQ((*tIterator).mGlobalID, kBlockDensityVector1.cbegin()->mGlobalID);
    ++tIterator;
    EXPECT_EQ((*tIterator).mDensity, std::next(kBlockDensityVector1.cbegin())->mDensity);
    EXPECT_EQ((*tIterator).mGlobalID, std::next(kBlockDensityVector1.cbegin())->mGlobalID);
}

TEST(MeshDesignVariablesViews, IteratorEqualityOperators)
{
    auto tIterator1 = MeshDesignVariablesDensitiesViewConstIterator{
        kSingleBlockDensities.cbegin(), kSingleBlockDensities.cend(), kBlockDensityVector1.cbegin()};
    auto tIterator2 = MeshDesignVariablesDensitiesViewConstIterator{
        kSingleBlockDensities.cbegin(), kSingleBlockDensities.cend(), kBlockDensityVector1.cbegin()};

    EXPECT_TRUE(tIterator1 == tIterator2) << "Explicitly check equality operator";
    EXPECT_FALSE(tIterator1 != tIterator2) << "Explicitly check inequality operator";

    ++tIterator1;
    EXPECT_FALSE(tIterator1 == tIterator2) << "Explicitly check equality operator";
    EXPECT_TRUE(tIterator1 != tIterator2) << "Explicitly check inequality operator";

    ++tIterator2;
    EXPECT_TRUE(tIterator1 == tIterator2) << "Explicitly check equality operator";
    EXPECT_FALSE(tIterator1 != tIterator2) << "Explicitly check inequality operator";
}

TEST(MeshDesignVariablesViews, Size)
{
    const auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"mars.exo", /*.mBlockDensities=*/kSingleBlockDensities};
    const auto tMeshView = MeshDesignVariablesDensitiesView{tMeshDesignVariables};

    ASSERT_EQ(tMeshView.size(), kDensities1.size());
}

TEST(MeshDesignVariablesViews, BeginEnd)
{
    const auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"jupiter.exo", /*.mBlockDensities=*/kSingleBlockDensities};
    const auto tMeshView = MeshDesignVariablesDensitiesView{tMeshDesignVariables};

    const auto tBeginIterator = tMeshView.begin();
    EXPECT_EQ(tBeginIterator.mOuterIterator, tMeshDesignVariables.mBlockDensities.cbegin());
    ASSERT_TRUE(tBeginIterator.mInnerIterator);
    assert(tBeginIterator.mInnerIterator);
    EXPECT_EQ(tBeginIterator.mInnerIterator.value(), tMeshDesignVariables.mBlockDensities.cbegin()->second.cbegin());
    EXPECT_EQ(tBeginIterator.mOuterIterator, tMeshDesignVariables.mBlockDensities.cbegin());
    EXPECT_EQ(tBeginIterator.mOuterIteratorEnd, tMeshDesignVariables.mBlockDensities.cend());

    const auto tEndIterator = tMeshView.end();
    EXPECT_FALSE(tEndIterator.mInnerIterator);
    EXPECT_EQ(tEndIterator.mOuterIterator, tMeshDesignVariables.mBlockDensities.cend());
    EXPECT_EQ(tEndIterator.mOuterIteratorEnd, tMeshDesignVariables.mBlockDensities.cend());

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
    const auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"venus.exo", /*.mBlockDensities=*/kSingleBlockDensities};
    const auto tMeshView = MeshDesignVariablesDensitiesView{tMeshDesignVariables};
    auto tDensityIterator = kBlockDensityVector1.cbegin();
    for (const auto tDensityValue : tMeshView)
    {
        EXPECT_EQ(tDensityValue.mDensity, tDensityIterator->mDensity);
        EXPECT_EQ(tDensityValue.mGlobalID, tDensityIterator->mGlobalID);
        ++tDensityIterator;
    }
}

TEST(MeshDesignVariablesViews, RangeBasedForTwoBlocks)
{
    const auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"pluto.exo", /*.mBlockDensities=*/kTwoBlockDensities};
    const auto tMeshView = MeshDesignVariablesDensitiesView{tMeshDesignVariables};
    auto tDensityIterator = kBlockDensityVector1.cbegin();
    for (const auto tDensityValue : tMeshView)
    {
        EXPECT_EQ(tDensityValue.mDensity, tDensityIterator->mDensity);
        EXPECT_EQ(tDensityValue.mGlobalID, tDensityIterator->mGlobalID);
        ++tDensityIterator;
        if (tDensityIterator == kBlockDensityVector1.cend())
        {
            tDensityIterator = kBlockDensityVector2.cbegin();
        }
    }
}

TEST(MeshDesignVariablesViews, MutableView)
{
    auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"uranus.exo", /*.mBlockDensities=*/kSingleBlockDensities};

    static_assert(!kIsConstIterator<std::vector<double>::iterator>);
    static_assert(kIsConstIterator<std::vector<double>::const_iterator>);

    const auto tMeshView = MeshDesignVariablesDensitiesMutableView{tMeshDesignVariables};

    const auto tNewDensities = std::vector{Density{0, 0.0}, Density{1, 1.0}, Density{2, 2.0}};
    std::copy(tNewDensities.cbegin(), tNewDensities.cend(), tMeshView.begin());

    const auto tVectorFromView =
        mesh_design_variables_to_vector(MeshDesignVariablesDensitiesView{tMeshDesignVariables});
    EXPECT_EQ(tVectorFromView, tNewDensities);
}

TEST(MeshDesignVariablesViews, ToVector)
{
    const auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"mercury.exo", /*.mBlockDensities=*/kSingleBlockDensities};
    const auto tVectorFromView =
        mesh_design_variables_to_vector(MeshDesignVariablesDensitiesView{tMeshDesignVariables});
    EXPECT_EQ(tVectorFromView, kBlockDensityVector1);
}

TEST(MeshDesignVariablesViews, ToVectorTwoBlocks)
{
    const auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"saturn.exo", /*.mBlockDensities=*/kTwoBlockDensities};
    const auto tVectorFromView =
        mesh_design_variables_to_vector(MeshDesignVariablesDensitiesView{tMeshDesignVariables});
    auto tAllDensities = kBlockDensityVector1;
    std::copy(kBlockDensityVector2.cbegin(), kBlockDensityVector2.cend(), std::back_inserter(tAllDensities));
    EXPECT_EQ(tVectorFromView, tAllDensities);
}

TEST(MeshDesignVariablesViews, CombineDensities)
{
    const auto tDensities = combine_densities_and_ids(kDensities1, kIDs1);
    EXPECT_EQ(tDensities, kBlockDensityVector1);
}

TEST(MeshDesignVariablesViews, SplitDensities)
{
    const auto [tDensityValues, tIDMap] = split_densities(kBlockDensityVector1);

    EXPECT_EQ(tDensityValues, kDensities1);
    EXPECT_EQ(tIDMap, kIDs1);
}
}  // namespace plato::mesh::unittest
