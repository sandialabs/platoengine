#include <gtest/gtest.h>

#include <cstddef>

#include "plato/third_party_integration/snopt/NonNullPtr.hpp"

namespace plato::third_party_integration::snopt::unittest
{
namespace
{
struct TestErrorPolicy
{
    static std::size_t mErrors;

    static void reset() { mErrors = 0U; }

    void operator()() { ++mErrors; }
};

std::size_t TestErrorPolicy::mErrors = 0U;

}  // namespace

TEST(NonNullPtr, ConstructionWithLocalVariableAddress)
{
    constexpr auto tArbitraryValue = 42;
    auto tData = tArbitraryValue;
    TestErrorPolicy::reset();
    [[maybe_unused]] const auto tPtr = NonNullPtr<int, TestErrorPolicy>{&tData};
    constexpr auto tExpectedNumberOfErrors = 0U;
    EXPECT_EQ(TestErrorPolicy::mErrors, tExpectedNumberOfErrors);
}

TEST(NonNullPtr, ConstructionWithNullPtr)
{
    int* const tData = nullptr;
    TestErrorPolicy::reset();
    [[maybe_unused]] const auto tPtr = NonNullPtr<int, TestErrorPolicy>{tData};
    constexpr auto tExpectedNumberOfErrors = 1U;
    EXPECT_EQ(TestErrorPolicy::mErrors, tExpectedNumberOfErrors);
}

TEST(NonNullPtr, ConstPtr)
{
    constexpr auto tArbitraryValue = 42;
    const auto tData = tArbitraryValue;
    auto tPtr = NonNullPtr{&tData};
    EXPECT_EQ(tPtr.get(), &tData);
    EXPECT_EQ(*tPtr.get(), tArbitraryValue);
}

TEST(NonNullPtr, Modification)
{
    constexpr auto tArbitraryValue = 42;
    auto tData = tArbitraryValue;
    auto tPtr = NonNullPtr{&tData};
    EXPECT_EQ(*tPtr.get(), tArbitraryValue);

    const auto tNewValue = 84U;
    *tPtr.get() = tNewValue;
    EXPECT_EQ(*tPtr.get(), tNewValue);
}

}  // namespace plato::third_party_integration::snopt::unittest
