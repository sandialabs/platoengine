#include <gtest/gtest.h>

#include "plato/analysis/AnalysisDomainMeshOperators.hpp"

namespace plato::analysis::unittest
{
TEST(AnalysisDomainMeshOperators, Equality)
{
    const auto tScalarFieldValue1 = ScalarFieldValue{10, 1, 42.0};
    const auto tScalarFieldValueSameAs1 = tScalarFieldValue1;

    EXPECT_EQ(tScalarFieldValue1, tScalarFieldValue1);
    EXPECT_EQ(tScalarFieldValue1, tScalarFieldValueSameAs1);

    // Test each entry different by 1
    const auto tScalarFieldValue2 = ScalarFieldValue{11, 1, 42.0};
    EXPECT_NE(tScalarFieldValue1, tScalarFieldValue2);

    const auto tScalarFieldValue3 = ScalarFieldValue{10, 2, 42.0};
    EXPECT_NE(tScalarFieldValue1, tScalarFieldValue3);

    const auto tScalarFieldValue4 = ScalarFieldValue{10, 1, 43.0};
    EXPECT_NE(tScalarFieldValue1, tScalarFieldValue4);
}
}  // namespace plato::analysis::unittest
