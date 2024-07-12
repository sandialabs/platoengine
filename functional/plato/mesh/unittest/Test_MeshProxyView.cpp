#include <gtest/gtest.h>

#include "plato/mesh/MeshProxy.hpp"
#include "plato/mesh/MeshProxyViews.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/MultidimensionalRange.hpp"

namespace plato::mesh::unittest
{
namespace
{
using MeshProxyDensitiesViewConstIterator =
    MeshProxyDensitiesViewIterator<std::vector<double>::const_iterator, std::forward_iterator_tag>;
}

TEST(MeshProxyViews, IncrementIterator)
{
    const auto tDensities = std::vector{42.0, 45.0, 48.0};
    auto tIterator = MeshProxyDensitiesViewConstIterator{tDensities.cbegin()};
    for (const auto [tIndex] : utilities::MultidimensionalRange{tDensities.size()})
    {
        EXPECT_EQ(tIterator.mIterator, std::next(tDensities.cbegin(), tIndex));
        const auto tReturnedIterator = ++tIterator;
        EXPECT_EQ(tReturnedIterator.mIterator, std::next(tDensities.cbegin(), tIndex + 1));
    }
}

TEST(MeshProxyViews, DereferenceIterator)
{
    const auto tDensities = std::vector{42.0, 43.0};
    auto tIterator = MeshProxyDensitiesViewConstIterator{tDensities.cbegin()};
    EXPECT_EQ(*tIterator, *tDensities.cbegin());
    ++tIterator;
    EXPECT_EQ(*tIterator, *std::next(tDensities.cbegin()));
}

TEST(MeshProxyViews, IteratorEqualityOperators)
{
    const auto tDensities = std::vector{100.0, 200.0};
    auto tIterator1 = MeshProxyDensitiesViewConstIterator{tDensities.cbegin()};
    auto tIterator2 = MeshProxyDensitiesViewConstIterator{tDensities.cbegin()};

    EXPECT_TRUE(tIterator1 == tIterator2) << "Explicitly check equality operator";
    EXPECT_FALSE(tIterator1 != tIterator2) << "Explicitly check inequality operator";

    ++tIterator1;
    EXPECT_FALSE(tIterator1 == tIterator2) << "Explicitly check equality operator";
    EXPECT_TRUE(tIterator1 != tIterator2) << "Explicitly check inequality operator";

    ++tIterator2;
    EXPECT_TRUE(tIterator1 == tIterator2) << "Explicitly check equality operator";
    EXPECT_FALSE(tIterator1 != tIterator2) << "Explicitly check inequality operator";
}

TEST(MeshProxyViews, SizeAndAccessor)
{
    const auto tDensities = std::vector{1.0, 2.0, 3.0};
    const auto tMeshProxy = MeshProxy{/*.mFileName=*/"mars.exo", /*.mNodalDensities=*/tDensities};
    const auto tMeshView = MeshProxyDensitiesView{tMeshProxy};

    ASSERT_EQ(tMeshView.size(), tDensities.size());
    for (const auto [tIndex] : utilities::MultidimensionalRange{tDensities.size()})
    {
        EXPECT_EQ(tMeshView[tIndex], tDensities[tIndex]);
    }
}

TEST(MeshProxyViews, BeginEnd)
{
    const auto tDensities = std::vector{0.0, 2.0, 4.0};
    const auto tMeshProxy = MeshProxy{/*.mFileName=*/"jupiter.exo", /*.mNodalDensities=*/tDensities};
    const auto tMeshView = MeshProxyDensitiesView{tMeshProxy};

    const auto tBeginIterator = tMeshView.begin();
    EXPECT_EQ(tBeginIterator.mIterator, tMeshProxy.mNodalDensities.cbegin());

    const auto tEndIterator = tMeshView.end();
    EXPECT_EQ(tEndIterator.mIterator, tMeshProxy.mNodalDensities.cend());
}

TEST(MeshProxyViews, RangeBasedFor)
{
    const auto tDensities = std::vector{-10.0, 20.0, 40.0};
    const auto tMeshProxy = MeshProxy{/*.mFileName=*/"venus.exo", /*.mNodalDensities=*/tDensities};
    const auto tMeshView = MeshProxyDensitiesView{tMeshProxy};
    auto tDensityIterator = tDensities.cbegin();
    for (const auto tDensityValue : tMeshView)
    {
        EXPECT_EQ(tDensityValue, *tDensityIterator);
        ++tDensityIterator;
    }
}

TEST(MeshProxyViews, ToVector)
{
    const auto tDensities = std::vector{1.0, 0.0, 4.0};
    const auto tMeshProxy = MeshProxy{/*.mFileName=*/"mercury.exo", /*.mNodalDensities=*/tDensities};
    const auto tVectorFromView = to_vector(MeshProxyDensitiesView{tMeshProxy});
    EXPECT_EQ(tVectorFromView, tDensities);
}

TEST(MeshProxyViews, MutableView)
{
    const auto tDensities = std::vector{1.0, 1.0, 1.0};
    auto tMeshProxy = MeshProxy{/*.mFileName=*/"uranus.exo", /*.mNodalDensities=*/tDensities};

    static_assert(!kIsConstIterator<std::vector<double>::iterator>);
    static_assert(kIsConstIterator<std::vector<double>::const_iterator>);

    const auto tMeshView = MeshProxyDensitiesMutableView{tMeshProxy};

    const auto tNewDensities = std::vector{0.0, 1.0, 2.0};
    std::copy(tNewDensities.cbegin(), tNewDensities.cend(), tMeshView.begin());

    for (const auto [tIndex] : utilities::MultidimensionalRange{tNewDensities.size()})
    {
        EXPECT_EQ(tMeshView[tIndex], tNewDensities[tIndex]);
    }
}

}  // namespace plato::mesh::unittest
