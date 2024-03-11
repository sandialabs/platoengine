#include <gtest/gtest.h>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <sstream>

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/DynamicVectorSerialization.hpp"

namespace plato::linear_algebra::unittest
{
TEST(DynamicVector, VectorConstructor)
{
    const auto tVector = std::vector{1.0, 2.0};
    const auto tDynamicVector = DynamicVector(tVector);

    EXPECT_EQ(tDynamicVector.size(), tVector.size());
    EXPECT_EQ(tDynamicVector[0], tVector[0]);
    EXPECT_EQ(tDynamicVector[1], tVector[1]);
}

TEST(DynamicVector, PlusEqual)
{
    const auto tVector1 = std::vector{1.0, 2.0, 3.0};
    const auto tDynamicVector1 = DynamicVector(tVector1);
    const auto tVector2 = std::vector{4.0, 5.0, 6.0};
    auto tDynamicVector2 = DynamicVector(tVector2);
    tDynamicVector2 += tDynamicVector1;

    for (std::size_t tIndex = 0; tIndex < tDynamicVector2.size(); ++tIndex)
    {
        EXPECT_EQ(tDynamicVector2[tIndex], tVector1[tIndex] + tVector2[tIndex]);
    }
}

TEST(DynamicVector, AdditionLValueLValue)
{
    const auto tVector1 = std::vector{1.0, 2.0, 3.0};
    const auto tDynamicVector1 = DynamicVector(tVector1);
    auto tDynamicVector2 = tDynamicVector1 + tDynamicVector1;

    for (std::size_t tIndex = 0; tIndex < tDynamicVector2.size(); ++tIndex)
    {
        EXPECT_EQ(tDynamicVector2[tIndex], 2.0 * tVector1[tIndex]);
    }
}

TEST(DynamicVector, AdditionRValueRValue)
{
    const auto tVector1 = std::vector{1.0, 2.0, 3.0};
    auto tDynamicVector1 = DynamicVector(tVector1);
    const auto tVector2 = std::vector{4.0, 5.0, 6.0};
    auto tDynamicVector2 = DynamicVector(tVector2);
    auto tDynamicVector3 = std::move(tDynamicVector1) + std::move(tDynamicVector2);

    for (std::size_t tIndex = 0; tIndex < tDynamicVector3.size(); ++tIndex)
    {
        EXPECT_EQ(tDynamicVector3[tIndex], tVector1[tIndex] + tVector2[tIndex]);
    }
}

TEST(DynamicVector, AdditionLValueRValue)
{
    const auto tVector1 = std::vector{1.0, 2.0, 3.0};
    const auto tDynamicVector1 = DynamicVector(tVector1);
    const auto tVector2 = std::vector{4.0, 5.0, 6.0};
    auto tDynamicVector2 = DynamicVector(tVector2);
    auto tDynamicVector3 = tDynamicVector1 + std::move(tDynamicVector2);

    for (std::size_t tIndex = 0; tIndex < tDynamicVector3.size(); ++tIndex)
    {
        EXPECT_EQ(tDynamicVector3[tIndex], tVector1[tIndex] + tVector2[tIndex]);
    }
}

TEST(DynamicVector, AdditionRValueLValue)
{
    const auto tVector1 = std::vector{1.0, 2.0, 3.0};
    auto tDynamicVector1 = DynamicVector(tVector1);
    const auto tVector2 = std::vector{4.0, 5.0, 6.0};
    const auto tDynamicVector2 = DynamicVector(tVector2);
    auto tDynamicVector3 = std::move(tDynamicVector1) + tDynamicVector2;

    for (std::size_t tIndex = 0; tIndex < tDynamicVector3.size(); ++tIndex)
    {
        EXPECT_EQ(tDynamicVector3[tIndex], tVector1[tIndex] + tVector2[tIndex]);
    }
}

TEST(DynamicVector, TimesEqual)
{
    const auto tVector = std::vector{1.0, 2.0, 3.0};
    auto tDynamicVector = DynamicVector(tVector);
    const auto tScalar = double{2.0};
    tDynamicVector *= tScalar;

    for (std::size_t tIndex = 0; tIndex < tDynamicVector.size(); ++tIndex)
    {
        EXPECT_EQ(tDynamicVector[tIndex], tScalar * tVector[tIndex]);
    }
}

TEST(DynamicVector, ScalarLeftMultiply)
{
    const auto tVector = std::vector{1.0, 2.0, 3.0};
    auto tDynamicVector1 = DynamicVector(tVector);
    const auto tScalar = double{2.0};
    const auto tDynamicVector2 = tScalar * std::move(tDynamicVector1);

    for (std::size_t tIndex = 0; tIndex < tDynamicVector2.size(); ++tIndex)
    {
        EXPECT_EQ(tDynamicVector2[tIndex], tScalar * tVector[tIndex]);
    }
}

TEST(DynamicVector, ScalarRightMultiply)
{
    const auto tVector = std::vector{1.0, 2.0, 3.0};
    auto tDynamicVector1 = DynamicVector(tVector);
    const auto tScalar = double{2.0};
    const auto tDynamicVector2 = std::move(tDynamicVector1) * tScalar;

    for (std::size_t tIndex = 0; tIndex < tDynamicVector2.size(); ++tIndex)
    {
        EXPECT_EQ(tDynamicVector2[tIndex], tScalar * tVector[tIndex]);
    }
}

TEST(DynamicVector, InnerProduct)
{
    const auto tVector1 = std::vector{1.0, 2.0, 3.0};
    const auto tDynamicVector1 = DynamicVector(tVector1);
    const auto tVector2 = std::vector{4.0, 5.0, 6.0};
    const auto tDynamicVector2 = DynamicVector(tVector2);
    const double tResult = tDynamicVector1.dot(tDynamicVector2);
    const double tExpected = std::inner_product(tVector1.cbegin(), tVector1.cend(), tVector2.cbegin(), 0.0);
    EXPECT_EQ(tResult, tExpected);
}

TEST(DynamicVector, StdVector)
{
    const auto tVector = std::vector{1, 2, 3};
    auto tDynamicVector = DynamicVector(tVector);
    const std::vector<int> tVectorCopy1 = tDynamicVector.stdVector();
    EXPECT_EQ(tVector, tVectorCopy1);

    const std::vector<int> tVectorCopy2 = std::move(tDynamicVector).stdVector();
    EXPECT_EQ(tVector, tVectorCopy2);
}

TEST(DynamicVector, Serialization)
{
    auto tStream = std::stringstream{};
    auto tSaveArchive = boost::archive::binary_oarchive{tStream};

    const auto tVector = std::vector{1.0, 2.0, 3.0, 4.0};
    const auto tStoredVector = DynamicVector(tVector);
    tSaveArchive << tStoredVector;

    auto tLoadedVector = DynamicVector<double>{};
    auto tLoadArchive = boost::archive::binary_iarchive{tStream};
    tLoadArchive >> tLoadedVector;
    for (std::size_t k = 0; k < tStoredVector.size(); ++k)
    {
        EXPECT_EQ(tStoredVector[k], tLoadedVector[k]);
    }
}
}  // namespace plato::linear_algebra::unittest
