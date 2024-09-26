#include <gtest/gtest.h>

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
    auto tData = 42;
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
    const auto tData = 42;
    auto tPtr = NonNullPtr{&tData};
    EXPECT_EQ(tPtr.get(), &tData);
    EXPECT_EQ(*tPtr.get(), 42);
}

TEST(NonNullPtr, Modification)
{
    auto tData = 42;
    auto tPtr = NonNullPtr{&tData};
    EXPECT_EQ(*tPtr.get(), 42U);

    const auto tNewValue = 84U;
    *tPtr.get() = tNewValue;
    EXPECT_EQ(*tPtr.get(), tNewValue);
}

}  // namespace plato::third_party_integration::snopt::unittest
