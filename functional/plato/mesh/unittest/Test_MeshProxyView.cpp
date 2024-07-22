#include <gtest/gtest.h>

#include <boost/core/ignore_unused.hpp>
#include <numeric>

#include "plato/mesh/MeshProxy.hpp"
#include "plato/mesh/MeshProxyViews.hpp"
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
const auto kBlockDensityVector1 =
    MeshProxy::DensityVector{{kIDs1[0], kDensities1[0]}, {kIDs1[1], kDensities1[1]}, {kIDs1[2], kDensities1[2]}};

const auto kDensities2 = std::vector{-1.0, 10.0};
const auto kBlockDensityVector2 = MeshProxy::DensityVector{{0, kDensities2[0]}, {1, kDensities2[1]}};

const auto kSingleBlockDensities = MeshProxy::BlockDensities{{0, kBlockDensityVector1}};
const auto kTwoBlockDensities = MeshProxy::BlockDensities{{0, kBlockDensityVector1}, {1, kBlockDensityVector2}};

using MeshProxyDensitiesViewConstIterator = MeshProxyDensitiesView::IteratorType;

}  // namespace

TEST(MeshProxyViews, IncrementIterator)
{
    auto tIterator = MeshProxyDensitiesViewConstIterator{kSingleBlockDensities.cbegin(), kSingleBlockDensities.cend(),
                                                         kBlockDensityVector1.cbegin()};
    for (const auto tIndex : utilities::IndexRange{kDensities1.size()})
    {
        boost::ignore_unused(tIndex);
        const auto tIteratorBeforeIncrement = tIterator;
        ++tIterator;
        const auto tDistance = std::distance(tIteratorBeforeIncrement, tIterator);
        EXPECT_EQ(tDistance, 1u);
    }
}

TEST(MeshProxyViews, DereferenceIterator)
{
    auto tIterator = MeshProxyDensitiesViewConstIterator{kSingleBlockDensities.cbegin(), kSingleBlockDensities.cend(),
                                                         kBlockDensityVector1.cbegin()};
    EXPECT_EQ((*tIterator).mDensity, kBlockDensityVector1.cbegin()->mDensity);
    EXPECT_EQ((*tIterator).mGlobalID, kBlockDensityVector1.cbegin()->mGlobalID);
    ++tIterator;
    EXPECT_EQ((*tIterator).mDensity, std::next(kBlockDensityVector1.cbegin())->mDensity);
    EXPECT_EQ((*tIterator).mGlobalID, std::next(kBlockDensityVector1.cbegin())->mGlobalID);
}

TEST(MeshProxyViews, IteratorEqualityOperators)
{
    auto tIterator1 = MeshProxyDensitiesViewConstIterator{kSingleBlockDensities.cbegin(), kSingleBlockDensities.cend(),
                                                          kBlockDensityVector1.cbegin()};
    auto tIterator2 = MeshProxyDensitiesViewConstIterator{kSingleBlockDensities.cbegin(), kSingleBlockDensities.cend(),
                                                          kBlockDensityVector1.cbegin()};

    EXPECT_TRUE(tIterator1 == tIterator2) << "Explicitly check equality operator";
    EXPECT_FALSE(tIterator1 != tIterator2) << "Explicitly check inequality operator";

    ++tIterator1;
    EXPECT_FALSE(tIterator1 == tIterator2) << "Explicitly check equality operator";
    EXPECT_TRUE(tIterator1 != tIterator2) << "Explicitly check inequality operator";

    ++tIterator2;
    EXPECT_TRUE(tIterator1 == tIterator2) << "Explicitly check equality operator";
    EXPECT_FALSE(tIterator1 != tIterator2) << "Explicitly check inequality operator";
}

TEST(MeshProxyViews, Size)
{
    const auto tMeshProxy = MeshProxy{/*.mFileName=*/"mars.exo", /*.mBlockDensities=*/kSingleBlockDensities};
    const auto tMeshView = MeshProxyDensitiesView{tMeshProxy};

    ASSERT_EQ(tMeshView.size(), kDensities1.size());
}

TEST(MeshProxyViews, BeginEnd)
{
    const auto tMeshProxy = MeshProxy{/*.mFileName=*/"jupiter.exo", /*.mBlockDensities=*/kSingleBlockDensities};
    const auto tMeshView = MeshProxyDensitiesView{tMeshProxy};

    const auto tBeginIterator = tMeshView.begin();
    EXPECT_EQ(tBeginIterator.mOuterIterator, tMeshProxy.mBlockDensities.cbegin());
    ASSERT_TRUE(tBeginIterator.mInnerIterator);
    assert(tBeginIterator.mInnerIterator);
    EXPECT_EQ(tBeginIterator.mInnerIterator.value(), tMeshProxy.mBlockDensities.cbegin()->second.cbegin());
    EXPECT_EQ(tBeginIterator.mOuterIterator, tMeshProxy.mBlockDensities.cbegin());
    EXPECT_EQ(tBeginIterator.mOuterIteratorEnd, tMeshProxy.mBlockDensities.cend());

    const auto tEndIterator = tMeshView.end();
    EXPECT_FALSE(tEndIterator.mInnerIterator);
    EXPECT_EQ(tEndIterator.mOuterIterator, tMeshProxy.mBlockDensities.cend());
    EXPECT_EQ(tEndIterator.mOuterIteratorEnd, tMeshProxy.mBlockDensities.cend());

    // Increment begin until it's at the end and check that they're equal
    auto tIncrementedIterator = tMeshView.begin();
    for (const auto tDensity : kDensities1)
    {
        boost::ignore_unused(tDensity);
        ++tIncrementedIterator;
    }
    EXPECT_EQ(tIncrementedIterator, tEndIterator);
}

TEST(MeshProxyViews, RangeBasedFor)
{
    const auto tMeshProxy = MeshProxy{/*.mFileName=*/"venus.exo", /*.mBlockDensities=*/kSingleBlockDensities};
    const auto tMeshView = MeshProxyDensitiesView{tMeshProxy};
    auto tDensityIterator = kBlockDensityVector1.cbegin();
    for (const auto tDensityValue : tMeshView)
    {
        EXPECT_EQ(tDensityValue.mDensity, tDensityIterator->mDensity);
        EXPECT_EQ(tDensityValue.mGlobalID, tDensityIterator->mGlobalID);
        ++tDensityIterator;
    }
}

TEST(MeshProxyViews, RangeBasedForTwoBlocks)
{
    const auto tMeshProxy = MeshProxy{/*.mFileName=*/"pluto.exo", /*.mBlockDensities=*/kTwoBlockDensities};
    const auto tMeshView = MeshProxyDensitiesView{tMeshProxy};
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

TEST(MeshProxyViews, MutableView)
{
    auto tMeshProxy = MeshProxy{/*.mFileName=*/"uranus.exo", /*.mBlockDensities=*/kSingleBlockDensities};

    static_assert(!kIsConstIterator<std::vector<double>::iterator>);
    static_assert(kIsConstIterator<std::vector<double>::const_iterator>);

    const auto tMeshView = MeshProxyDensitiesMutableView{tMeshProxy};

    const auto tNewDensities = std::vector{Density{0, 0.0}, Density{1, 1.0}, Density{2, 2.0}};
    std::copy(tNewDensities.cbegin(), tNewDensities.cend(), tMeshView.begin());

    const auto tVectorFromView = mesh_proxy_to_vector(MeshProxyDensitiesView{tMeshProxy});
    EXPECT_EQ(tVectorFromView, tNewDensities);
}

TEST(MeshProxyViews, ToVector)
{
    const auto tMeshProxy = MeshProxy{/*.mFileName=*/"mercury.exo", /*.mBlockDensities=*/kSingleBlockDensities};
    const auto tVectorFromView = mesh_proxy_to_vector(MeshProxyDensitiesView{tMeshProxy});
    EXPECT_EQ(tVectorFromView, kBlockDensityVector1);
}

TEST(MeshProxyViews, ToVectorTwoBlocks)
{
    const auto tMeshProxy = MeshProxy{/*.mFileName=*/"saturn.exo", /*.mBlockDensities=*/kTwoBlockDensities};
    const auto tVectorFromView = mesh_proxy_to_vector(MeshProxyDensitiesView{tMeshProxy});
    auto tAllDensities = kBlockDensityVector1;
    std::copy(kBlockDensityVector2.cbegin(), kBlockDensityVector2.cend(), std::back_inserter(tAllDensities));
    EXPECT_EQ(tVectorFromView, tAllDensities);
}

TEST(MeshProxyViews, CombineDensities)
{
    const auto tDensities = combine_densities_and_ids(kDensities1, kIDs1);
    EXPECT_EQ(tDensities, kBlockDensityVector1);
}

TEST(MeshProxyViews, SplitDensities)
{
    const auto [tDensityValues, tIDMap] = split_densities(kBlockDensityVector1);

    EXPECT_EQ(tDensityValues, kDensities1);
    EXPECT_EQ(tIDMap, kIDs1);
}
}  // namespace plato::mesh::unittest
