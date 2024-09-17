#include <gtest/gtest.h>

#include <boost/core/ignore_unused.hpp>

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialViewIterator.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::analysis::unittest
{
namespace
{
const auto kScalarField = std::vector{4.0, 2.0, 7.0};
const auto kIDs = std::vector<std::size_t>{0, 1, 2};
const auto kBlockScalarFieldVector = AnalysisDomainMesh::ScalarFieldVector{
    {kIDs[0], kIDs[0], kScalarField[0]}, {kIDs[1], kIDs[1], kScalarField[1]}, {kIDs[2], kIDs[2], kScalarField[2]}};

using AnalysisDomainMeshSequentialViewConstIterator = AnalysisDomainMeshSequentialView::IteratorType;
}  // namespace

TEST(AnalysisDomainMeshViews, IncrementIterator)
{
    auto tIterator = AnalysisDomainMeshSequentialViewConstIterator{{kBlockScalarFieldVector.cbegin()},
                                                                   {kBlockScalarFieldVector.cend()}};
    for (const auto tIndex : utilities::IndexRange{kScalarField.size()})
    {
        boost::ignore_unused(tIndex);
        const auto tIteratorBeforeIncrement = tIterator;
        ++tIterator;
        const auto tDistance = std::distance(tIteratorBeforeIncrement, tIterator);
        EXPECT_EQ(tDistance, 1u);
    }
}

TEST(AnalysisDomainMeshViews, DereferenceIterator)
{
    auto tIterator = AnalysisDomainMeshSequentialViewConstIterator{{kBlockScalarFieldVector.cbegin()},
                                                                   {kBlockScalarFieldVector.cend()}};
    EXPECT_EQ(static_cast<ScalarFieldValue>(*tIterator).mValue, kBlockScalarFieldVector.cbegin()->mValue);
    EXPECT_EQ(static_cast<ScalarFieldValue>(*tIterator).mGlobalMeshEntityID,
              kBlockScalarFieldVector.cbegin()->mGlobalMeshEntityID);
    ++tIterator;
    EXPECT_EQ(static_cast<ScalarFieldValue>(*tIterator).mValue, std::next(kBlockScalarFieldVector.cbegin())->mValue);
    EXPECT_EQ(static_cast<ScalarFieldValue>(*tIterator).mGlobalMeshEntityID,
              std::next(kBlockScalarFieldVector.cbegin())->mGlobalMeshEntityID);
}

TEST(AnalysisDomainMeshViews, IteratorEqualityOperators)
{
    auto tIterator1 = AnalysisDomainMeshSequentialViewConstIterator{{kBlockScalarFieldVector.cbegin()},
                                                                    {kBlockScalarFieldVector.cend()}};
    auto tIterator2 = AnalysisDomainMeshSequentialViewConstIterator{{kBlockScalarFieldVector.cbegin()},
                                                                    {kBlockScalarFieldVector.cend()}};

    EXPECT_TRUE(tIterator1 == tIterator2) << "Explicitly check equality operator";
    EXPECT_FALSE(tIterator1 != tIterator2) << "Explicitly check inequality operator";

    ++tIterator1;
    EXPECT_FALSE(tIterator1 == tIterator2) << "Explicitly check equality operator";
    EXPECT_TRUE(tIterator1 != tIterator2) << "Explicitly check inequality operator";

    ++tIterator2;
    EXPECT_TRUE(tIterator1 == tIterator2) << "Explicitly check equality operator";
    EXPECT_FALSE(tIterator1 != tIterator2) << "Explicitly check inequality operator";
}
}  // namespace plato::analysis::unittest
