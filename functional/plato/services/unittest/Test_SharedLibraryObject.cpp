#include <gtest/gtest.h>

#include <cmath>
#include <memory>
#include <string_view>
#include <utility>

#include "plato/services/SharedLibraryObject.hpp"
#include "plato/services/SharedLibrarySetupTeardown.hpp"
#include "plato/services/test_shared_lib/TestSharedLib.hpp"
#include "plato/test_utilities/CoutCerrPrintTestFixture.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::services::unittest
{
namespace
{
constexpr auto kSharedLibName = std::string_view{"libPlatoServicesTestSharedLib.so"};
constexpr auto kLibraryLoadFunctionName = std::string_view{"load_library"};
constexpr auto kLibraryUnloadFunctionName = std::string_view{"unload_library"};
constexpr auto kCreateFunction = std::string_view{"create_test_interface"};
constexpr auto kDoubleValue = 42.2;

using SharedLibraryObjectCoutRedirectFixture = test_utilities::CoutCerrPrintTestFixture;

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

TEST_F(SharedLibraryObjectCoutRedirectFixture, LibraryLoadUnload)
{
    {
        [[maybe_unused]] const auto tSharedLib =
            make_shared_library_object<std::unique_ptr<test_shared_lib::TestInterface>(double)>(
                SharedLibraryData{.mSharedLibraryPath = kSharedLibName,
                                  .mLibraryLoadFunctionName = std::string{kLibraryLoadFunctionName},
                                  .mLibraryShutdownFunctionName = std::string{kLibraryUnloadFunctionName}},
                kCreateFunction, kDoubleValue);
    }
    const auto tExpectedOutput = std::string{"Test library was loaded.\nTest library was unloaded.\n"};
    checkRankZeroCoutStringStream(tExpectedOutput, TEST_CONTEXT("Library unload"));
}

}  // namespace plato::services::unittest
