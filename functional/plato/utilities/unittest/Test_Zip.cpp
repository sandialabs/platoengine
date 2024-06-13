#include <gtest/gtest.h>

#include <list>
#include <numeric>
#include <set>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/Zip.hpp"
#include "plato/utilities/ZipIterator.hpp"

namespace plato::utilities::unittest
{
namespace
{
struct CopyCounter
{
    unsigned int mCount = 0;
    CopyCounter() {}
    CopyCounter(const CopyCounter& aCopy) : mCount{aCopy.mCount} { ++mCount; }
};
}  // namespace

TEST(Zip, AnyOf)
{
    constexpr auto tTuple1 = std::make_tuple(1, 'a');
    constexpr auto tTuple2 = std::make_tuple(2, 'a');
    constexpr auto tTuple3 = std::make_tuple(1, 'b');
    constexpr auto tTuple4 = std::make_tuple(2, 'b');

    EXPECT_TRUE(any_of_comparison(tTuple1, tTuple1, std::equal_to{}));
    EXPECT_TRUE(any_of_comparison(tTuple1, tTuple2, std::equal_to{}));
    EXPECT_TRUE(any_of_comparison(tTuple1, tTuple3, std::equal_to{}));
    EXPECT_FALSE(any_of_comparison(tTuple1, tTuple4, std::equal_to{}));
}

TEST(Zip, IteratorsEqual)
{
    const auto tVector1 = std::vector<int>{1, 2, 3};
    const auto tVector2 = std::vector<char>{'a', 'b', 'c'};
    const auto tIter1 = ZipIterator{tVector1.cbegin(), tVector2.cbegin()};
    const auto tIter2 = ZipIterator{std::make_tuple(tVector1.cend(), tVector2.cend())};

    EXPECT_EQ(tIter1, tIter1);
    EXPECT_NE(tIter1, tIter2);
}

TEST(Zip, ExplicitIteratorType)
{
    const auto tVector1 = std::vector<int>{1, 2, 3};
    auto tVector2 = std::vector<char>{'a', 'b', 'c'};
    using ZipIteratorType = ZipIterator<std::vector<int>::const_iterator, std::vector<char>::iterator>;
    const auto tIter = ZipIteratorType{tVector1.begin(), tVector2.begin()};
    EXPECT_EQ(tIter, tIter);
}

TEST(Zip, DereferenceIterator)
{
    const auto tVector1 = std::vector<int>{1, 2, 3};
    const auto tVector2 = std::vector<char>{'a', 'b', 'c'};

    {
        const auto tIter = ZipIterator{tVector1.cbegin(), tVector2.cbegin()};
        EXPECT_EQ(std::get<0>(*tIter), 1);
        EXPECT_EQ(std::get<1>(*tIter), 'a');
    }

    {
        const auto tIter = ZipIterator{std::next(tVector1.cbegin()), std::next(tVector2.cbegin())};
        EXPECT_EQ(std::get<0>(*tIter), 2);
        EXPECT_EQ(std::get<1>(*tIter), 'b');
    }

    {
        const auto tIter = ZipIterator{tVector1.crbegin(), tVector2.crbegin()};
        EXPECT_EQ(std::get<0>(*tIter), 3);
        EXPECT_EQ(std::get<1>(*tIter), 'c');
    }

    {
        const auto tIter = ZipIterator{tVector1.cbegin(), tVector2.crbegin()};
        EXPECT_EQ(std::get<0>(*tIter), 1);
        EXPECT_EQ(std::get<1>(*tIter), 'c');
    }
}

TEST(Zip, DereferencingIteratorDoesNotCopy)
{
    auto tVector1 = std::vector<CopyCounter>{};
    tVector1.reserve(2);
    tVector1.emplace_back();
    tVector1.emplace_back();
    EXPECT_EQ(tVector1.front().mCount, 0);
    {
        const auto tIter = tVector1.cbegin();
        EXPECT_EQ(tIter->mCount, 0);
    }
    {
        const auto tZipIterator = ZipIterator{tVector1.cbegin()};
        const auto& value = *tZipIterator;
        EXPECT_EQ(std::get<0>(value).mCount, 0);
    }

    // Copies these on construction, so expect 1 copy
    auto tVector2 = std::vector<CopyCounter>(3, CopyCounter{});
    {
        auto tZipIterator = ZipIterator{tVector1.rbegin(), tVector2.begin()};
        auto values = *tZipIterator;
        EXPECT_EQ(std::get<0>(values).mCount, 0);
        EXPECT_EQ(std::get<1>(values).mCount, 1);
    }
    {
        auto tZipIterator = ZipIterator{tVector1.rbegin(), tVector2.begin()};
        auto [tValue1, tValue2] = *tZipIterator;
        EXPECT_EQ(tValue1.mCount, 0);
        EXPECT_EQ(tValue2.mCount, 1);
    }
}

TEST(Zip, ModifyThroughIterator)
{
    auto tVector1 = std::vector<int>{1, 2, 3};
    auto tVector2 = std::vector<char>{'a', 'b', 'c'};
    auto tZipIterator = ZipIterator{tVector1.begin(), tVector2.rbegin()};

    auto [tValue1, tValue2] = *tZipIterator;
    tValue1 = 42;
    EXPECT_EQ(tVector1.front(), 42);
    tValue2 = 'z';
    EXPECT_EQ(tVector2.back(), 'z');
}

TEST(Zip, Increment)
{
    const auto tVector1 = std::vector<int>{1, 2, 3};
    const auto tVector2 = std::vector<char>{'a', 'b', 'c'};
    auto tZipIterator = ZipIterator{tVector1.cbegin(), tVector2.crbegin()};

    {
        const auto [tValue1, tValue2] = *tZipIterator;
        EXPECT_EQ(tValue1, 1);
        EXPECT_EQ(tValue2, 'c');
    }
    ++tZipIterator;
    {
        const auto [tValue1, tValue2] = *tZipIterator;
        EXPECT_EQ(tValue1, 2);
        EXPECT_EQ(tValue2, 'b');
    }
    ++tZipIterator;
    {
        const auto [tValue1, tValue2] = *tZipIterator;
        EXPECT_EQ(tValue1, 3);
        EXPECT_EQ(tValue2, 'a');
    }
}

TEST(Zip, StdAccumulate)
{
    const auto tVector1 = std::vector<int>{1, 2, 3};
    const auto tVector2 = std::vector<int>{10, 11, 12, 13};
    auto tZipBegin = ZipIterator{tVector1.cbegin(), tVector2.crbegin()};
    auto tZipEnd = ZipIterator{tVector1.cend(), tVector2.crend()};
    const auto tResult = std::accumulate(tZipBegin, tZipEnd, 0,
                                         [](const int aSum, const auto& aValues)
                                         { return std::get<0>(aValues) + std::get<1>(aValues) + aSum; });
    EXPECT_EQ(tResult, 42);
}

TEST(Zip, ZipTypes)
{
    using Vector1 = std::vector<int>;
    using Vector2 = std::vector<char>;
    using TestZip = Zip<Vector1, Vector2>;

    auto tVector1 = Vector1{1, 2, 3};
    auto tVector2 = Vector2{'a', 'b', 'c'};

    const auto tTestTypesAndBeginIters = [](auto&& aTestZip, const test_utilities::TestContext& aTestContext)
    {
        auto tBeginIter = aTestZip.begin();
        using BeginIter = decltype(tBeginIter);
        EXPECT_TRUE((std::is_same_v<BeginIter, ZipIterator<Vector1::iterator, Vector2::iterator>>)) << aTestContext;

        EXPECT_EQ(std::get<0>(*tBeginIter), 1) << aTestContext;
        EXPECT_EQ(std::get<1>(*tBeginIter), 'a') << aTestContext;
    };
    // Both lvalues for arguments
    {
        auto tTestZip = TestZip{tVector1, tVector2};
        tTestTypesAndBeginIters(tTestZip, TEST_CONTEXT("Both lvalue arguments"));
    }
    // Temporary as one argument
    {
        auto tTestZip = TestZip{std::vector<int>{1, 2, 3}, tVector2};
        tTestTypesAndBeginIters(tTestZip, TEST_CONTEXT("Temporary as one argument"));
    }
    // Deduced
    {
        const auto tVector3 = Vector1{1, 2, 3};
        auto tTestZip = Zip{tVector1, tVector3};
        auto tBeginIter = tTestZip.begin();
        using BeginIter = decltype(tBeginIter);
        EXPECT_TRUE((std::is_same_v<BeginIter, ZipIterator<Vector1::iterator, Vector1::const_iterator>>));
    }
}

TEST(Zip, ZipBegin)
{
    auto tVector1 = std::vector<int>{1, 2, 3};
    auto tVector2 = std::vector<char>{'a', 'b', 'c'};
    auto tZip = Zip{tVector1, tVector2};
    auto tBeginIter = tZip.begin();
    EXPECT_EQ(std::get<0>(*tBeginIter), 1);
    EXPECT_EQ(std::get<1>(*tBeginIter), 'a');
}

TEST(Zip, ZipBeginTemporary)
{
    const auto tAlphabet = std::string{"abcdefghijklmnopqrstuvwxyz"};
    const auto tDigits = std::string{"1234567890"};
    auto tVector1 = std::vector<int>{1, 2};
    auto tZip = Zip{tVector1, std::vector<std::string>{tAlphabet, tDigits}};
    auto tBeginIter = tZip.begin();
    EXPECT_EQ(std::get<0>(*tBeginIter), 1);
    EXPECT_EQ(std::get<1>(*tBeginIter), tAlphabet);
    ++tBeginIter;
    EXPECT_EQ(std::get<0>(*tBeginIter), 2);
    EXPECT_EQ(std::get<1>(*tBeginIter), tDigits);
}

TEST(Zip, ZipBeginConst)
{
    const auto tVector1 = std::vector<int>{1, 2, 3};
    const auto tVector2 = std::vector<char>{'a', 'b', 'c'};
    const auto tZip = Zip{tVector1, tVector2};
    auto tBeginIter = tZip.begin();
    EXPECT_EQ(std::get<0>(*tBeginIter), 1);
    EXPECT_EQ(std::get<1>(*tBeginIter), 'a');
}

TEST(Zip, ZipEnd)
{
    const auto tVector1 = std::vector<int>{1, 2, 3};
    const auto tVector2 = std::vector<char>{'a', 'b', 'c'};
    const auto tZip = Zip{tVector1, tVector2};
    auto tEndIter = tZip.end();
    EXPECT_EQ(tEndIter, (ZipIterator{tVector1.end(), tVector2.end()}));
}

TEST(Zip, VectorsSameLengthConst)
{
    const auto tDoubleVector = std::vector<double>{1.0, 2.0, 3.0};
    const auto tCharVector = std::vector<char>{'a', 'b', 'c'};

    auto tResultDoubleVector = std::vector<double>{};
    auto tResultCharVector = std::vector<char>{};
    for (const auto [tValue1, tValue2] : Zip{tDoubleVector, tCharVector})
    {
        tResultDoubleVector.push_back(tValue1);
        tResultCharVector.push_back(tValue2);
    }
    EXPECT_EQ(tDoubleVector, tResultDoubleVector);
    EXPECT_EQ(tCharVector, tResultCharVector);
}

TEST(Zip, VectorsSameLengthModifying)
{
    auto tDoubleVector = std::vector<double>{1.0, 2.0, 3.0};
    auto tIntVector = std::vector<int>{4, 5, 6};
    auto tCount = int{100};
    for (auto [tValue1, tValue2] : Zip{tDoubleVector, tIntVector})
    {
        tValue1 = tCount;
        tValue2 = 2 * tCount;
        ++tCount;
    }
    const auto tExpectedDoubleVector = std::vector<double>{100.0, 101.0, 102.0};
    const auto tExpectedIntVector = std::vector<int>{200, 202, 204};
    EXPECT_EQ(tDoubleVector, tExpectedDoubleVector);
    EXPECT_EQ(tIntVector, tExpectedIntVector);
}

TEST(Zip, VectorsDifferentLengthTemporary)
{
    auto tDoubleVector = std::vector<double>{1.0, 2.0, 3.0};
    auto tResultDoubleVector = std::vector<double>{};
    auto tResultIntVector = std::vector<int>{};
    for (const auto [tValue1, tValue2] : Zip{tDoubleVector, std::vector<int>{4, 5, 6, 7}})
    {
        tResultDoubleVector.push_back(tValue1);
        tResultIntVector.push_back(tValue2);
    }
    EXPECT_EQ(tDoubleVector, tResultDoubleVector);
    EXPECT_EQ((std::vector<int>{4, 5, 6}), tResultIntVector);
}

TEST(Zip, ListAndSet)
{
    const auto tIntSet = std::set<int>{1, 2, 3, 4, 4, 4};
    const auto tCharList = std::list<char>{'a', 'b', 'c', 'd', 'e'};
    auto tResultInts = std::vector<int>{};
    auto tResultChars = std::vector<char>{};
    for (const auto [int_value, char_value] : Zip{tIntSet, tCharList})
    {
        tResultInts.push_back(int_value);
        tResultChars.push_back(char_value);
    }
    const auto tExpectedInts = std::vector<int>{1, 2, 3, 4};
    const auto tExpectedChars = std::vector<char>{'a', 'b', 'c', 'd'};
    EXPECT_EQ(tResultInts, tExpectedInts);
    EXPECT_EQ(tResultChars, tExpectedChars);
}

TEST(Zip, NoCopiesForLValues)
{
    auto tVector1 = std::vector<CopyCounter>{};
    tVector1.reserve(2);
    tVector1.emplace_back();
    tVector1.emplace_back();
    auto tCopyCount = unsigned{0u};
    for (const auto [tValue1] : Zip{tVector1})
    {
        tCopyCount += tValue1.mCount;
    }
    EXPECT_EQ(tCopyCount, 0);
}

TEST(Zip, CopiesForRValues)
{
    auto tCopyCount = unsigned{0u};
    for (const auto [tValue1] : Zip{std::vector<CopyCounter>(3)})
    {
        tCopyCount += tValue1.mCount;
    }
    EXPECT_EQ(tCopyCount, 3);
}

}  // namespace plato::utilities::unittest
