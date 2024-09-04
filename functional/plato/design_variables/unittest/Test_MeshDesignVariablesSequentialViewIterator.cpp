#include <gtest/gtest.h>

#include <boost/core/ignore_unused.hpp>

#include "plato/design_variables/MeshDesignVariablesSequentialView.hpp"
#include "plato/design_variables/MeshDesignVariablesSequentialViewIterator.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::design_variables::unittest
{
namespace
{
const auto kScalarField = std::vector{4.0, 2.0, 7.0};
const auto kIDs = std::vector<std::size_t>{0, 1, 2};
const auto kBlockScalarFieldVector = MeshDesignVariables::ScalarFieldVector{
    {kIDs[0], kIDs[0], kScalarField[0]}, {kIDs[1], kIDs[1], kScalarField[1]}, {kIDs[2], kIDs[2], kScalarField[2]}};

using MeshDesignVariablesSequentialViewConstIterator = MeshDesignVariablesSequentialView::IteratorType;
}  // namespace

TEST(MeshDesignVariablesViews, IncrementIterator)
{
    auto tIterator = MeshDesignVariablesSequentialViewConstIterator{{kBlockScalarFieldVector.cbegin()},
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

TEST(MeshDesignVariablesViews, DereferenceIterator)
{
    auto tIterator = MeshDesignVariablesSequentialViewConstIterator{{kBlockScalarFieldVector.cbegin()},
                                                                    {kBlockScalarFieldVector.cend()}};
    EXPECT_EQ(static_cast<ScalarFieldValue>(*tIterator).mValue, kBlockScalarFieldVector.cbegin()->mValue);
    EXPECT_EQ(static_cast<ScalarFieldValue>(*tIterator).mGlobalMeshEntityID,
              kBlockScalarFieldVector.cbegin()->mGlobalMeshEntityID);
    ++tIterator;
    EXPECT_EQ(static_cast<ScalarFieldValue>(*tIterator).mValue, std::next(kBlockScalarFieldVector.cbegin())->mValue);
    EXPECT_EQ(static_cast<ScalarFieldValue>(*tIterator).mGlobalMeshEntityID,
              std::next(kBlockScalarFieldVector.cbegin())->mGlobalMeshEntityID);
}

TEST(MeshDesignVariablesViews, IteratorEqualityOperators)
{
    auto tIterator1 = MeshDesignVariablesSequentialViewConstIterator{{kBlockScalarFieldVector.cbegin()},
                                                                     {kBlockScalarFieldVector.cend()}};
    auto tIterator2 = MeshDesignVariablesSequentialViewConstIterator{{kBlockScalarFieldVector.cbegin()},
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
}  // namespace plato::design_variables::unittest
