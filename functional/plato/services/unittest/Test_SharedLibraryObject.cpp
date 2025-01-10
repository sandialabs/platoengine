#include <gtest/gtest.h>

#include <cmath>

#include "plato/services/SharedLibraryObject.hpp"
#include "plato/services/test_shared_lib/TestSharedLib.hpp"

namespace plato::services::unittest
{
namespace
{
constexpr auto kSharedLibName = "libPlatoServicesTestSharedLib.so";
constexpr auto kDoubleValue = double{42.2};

auto make_test_object() -> SharedLibraryObject<std::unique_ptr<test_shared_lib::TestInterface>>
{
    auto tSharedLib = SharedLibrarySetupTeardown{kSharedLibName};
    constexpr auto tRoundFunction = std::string_view{"create_test_interface"};
    return SharedLibraryObject<std::unique_ptr<test_shared_lib::TestInterface>>{
        std::move(tSharedLib),
        tSharedLib.call<std::unique_ptr<test_shared_lib::TestInterface>(double)>(tRoundFunction, kDoubleValue)};
}
}  // namespace

TEST(SharedLibraryObject, Creation)
{
    const auto tSharedLib = make_test_object();
    const auto tExpected = std::lround(kDoubleValue);
    EXPECT_EQ(tExpected, tSharedLib.object()->testRound());
}
}  // namespace plato::services::unittest
