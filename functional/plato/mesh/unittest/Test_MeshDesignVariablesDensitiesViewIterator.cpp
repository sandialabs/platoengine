#include <gtest/gtest.h>

#include <boost/core/ignore_unused.hpp>

#include "plato/mesh/MeshDesignVariablesDensitiesView.hpp"
#include "plato/mesh/MeshDesignVariablesDensitiesViewIterator.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::mesh::unittest
{
namespace
{
const auto kDensities = std::vector{4.0, 2.0, 7.0};
const auto kIDs = std::vector<std::size_t>{0, 1, 2};
const auto kBlockDensityVector = MeshDesignVariables::DensityVector{
    {kIDs[0], kIDs[0], kDensities[0]}, {kIDs[1], kIDs[1], kDensities[1]}, {kIDs[2], kIDs[2], kDensities[2]}};

using MeshDesignVariablesDensitiesViewConstIterator = MeshDesignVariablesDensitiesView::IteratorType;
}  // namespace

TEST(MeshDesignVariablesViews, IncrementIterator)
{
    auto tIterator =
        MeshDesignVariablesDensitiesViewConstIterator{{kBlockDensityVector.cbegin()}, {kBlockDensityVector.cend()}};
    for (const auto tIndex : utilities::IndexRange{kDensities.size()})
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
    auto tIterator =
        MeshDesignVariablesDensitiesViewConstIterator{{kBlockDensityVector.cbegin()}, {kBlockDensityVector.cend()}};
    EXPECT_EQ(static_cast<Density>(*tIterator).mDensity, kBlockDensityVector.cbegin()->mDensity);
    EXPECT_EQ(static_cast<Density>(*tIterator).mGlobalMeshEntityID, kBlockDensityVector.cbegin()->mGlobalMeshEntityID);
    ++tIterator;
    EXPECT_EQ(static_cast<Density>(*tIterator).mDensity, std::next(kBlockDensityVector.cbegin())->mDensity);
    EXPECT_EQ(static_cast<Density>(*tIterator).mGlobalMeshEntityID,
              std::next(kBlockDensityVector.cbegin())->mGlobalMeshEntityID);
}

TEST(MeshDesignVariablesViews, IteratorEqualityOperators)
{
    auto tIterator1 =
        MeshDesignVariablesDensitiesViewConstIterator{{kBlockDensityVector.cbegin()}, {kBlockDensityVector.cend()}};
    auto tIterator2 =
        MeshDesignVariablesDensitiesViewConstIterator{{kBlockDensityVector.cbegin()}, {kBlockDensityVector.cend()}};

    EXPECT_TRUE(tIterator1 == tIterator2) << "Explicitly check equality operator";
    EXPECT_FALSE(tIterator1 != tIterator2) << "Explicitly check inequality operator";

    ++tIterator1;
    EXPECT_FALSE(tIterator1 == tIterator2) << "Explicitly check equality operator";
    EXPECT_TRUE(tIterator1 != tIterator2) << "Explicitly check inequality operator";

    ++tIterator2;
    EXPECT_TRUE(tIterator1 == tIterator2) << "Explicitly check equality operator";
    EXPECT_FALSE(tIterator1 != tIterator2) << "Explicitly check inequality operator";
}
}  // namespace plato::mesh::unittest
