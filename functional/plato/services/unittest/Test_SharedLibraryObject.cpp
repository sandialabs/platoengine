#include <gtest/gtest.h>

#include <cmath>

#include "plato/services/SharedLibraryObject.hpp"
#include "plato/services/test_shared_lib/TestSharedLib.hpp"

namespace plato::services::unittest
{
namespace
{
constexpr auto kSharedLibName = std::string_view{"libPlatoServicesTestSharedLib.so"};
constexpr auto kCreateFunction = std::string_view{"create_test_interface"};
constexpr auto kDoubleValue = double{42.2};

auto make_test_object() -> SharedLibraryObject<std::unique_ptr<test_shared_lib::TestInterface>>
{
    auto tSharedLib = SharedLibrarySetupTeardown{kSharedLibName};
    return SharedLibraryObject<std::unique_ptr<test_shared_lib::TestInterface>>{
        std::move(tSharedLib),
        tSharedLib.call<std::unique_ptr<test_shared_lib::TestInterface>(double)>(kCreateFunction, kDoubleValue)};
}
}  // namespace

TEST(SharedLibraryObject, CreationViaCtor)
{
    const auto tSharedLib = make_test_object();
    const auto tExpected = std::lround(kDoubleValue);
    EXPECT_EQ(tExpected, tSharedLib.object()->testRound());
}

TEST(SharedLibraryObject, CreationViaFunction)
{
    const auto tSharedLib = make_shared_library_object<std::unique_ptr<test_shared_lib::TestInterface>(double)>(
        kSharedLibName, kCreateFunction, kDoubleValue);
    const auto tExpected = std::lround(kDoubleValue);
    EXPECT_EQ(tExpected, tSharedLib.object()->testRound());
}

}  // namespace plato::services::unittest
