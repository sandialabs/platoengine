#include <gtest/gtest.h>

#include <functional>
#include <vector>

#include "plato/main/library/Executor.hpp"

namespace plato::main::library::unittest
{
namespace
{
using DummyFunctionNonConstArg = std::function<void(int&)>;
using DummyFunctionConstArg = std::function<void(const int&)>;

}  // namespace

TEST(Executor, DoubleDummyProcessManager)
{
    static auto tDummyFunctionCallCount = 0;
    const auto tDummyProcessManager = DummyFunctionConstArg{[](const int&) { ++tDummyFunctionCallCount; }};

    const auto tExecutor = Executor{std::vector{tDummyProcessManager, tDummyProcessManager}};
    const auto tDummyArg = 0;
    tExecutor.execute(tDummyArg);
    EXPECT_EQ(tDummyFunctionCallCount, 2);
}

TEST(Executor, DummyProcessManagerNonConstArg)
{
    const auto tDummyProcessManager = DummyFunctionNonConstArg{[](int& aArg) { ++aArg; }};

    const auto tExecutor = Executor{std::vector{tDummyProcessManager}};
    auto tDummyArg = 0;
    tExecutor.execute(tDummyArg);
    EXPECT_EQ(tDummyArg, 1);
}

}  // namespace plato::main::library::unittest
