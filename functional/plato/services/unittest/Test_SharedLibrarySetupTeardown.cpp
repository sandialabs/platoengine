#include <gtest/gtest.h>

#include <cmath>
#include <optional>
#include <string_view>
#include <utility>

#include "plato/services/SharedLibrarySetupTeardown.hpp"
#include "plato/test_utilities/CoutCerrPrintTestFixture.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::services::unittest
{
namespace
{
constexpr auto kSharedLibraryName = "libPlatoServicesTestSharedLib.so";
constexpr auto kBogusSharedLibraryName = "libPlatoServicesTestBogusSharedLib.so";
constexpr auto kLibraryLoadFunctionName = "load_library";
constexpr auto kLibraryUnloadFunctionName = "unload_library";
constexpr auto kDoubleValue = 42.2;

using SharedLibraryCoutRedirectFixture = test_utilities::CoutCerrPrintTestFixture;

void check_shared_lib_function(SharedLibrarySetupTeardown& aSharedLibrary,
                               const test_utilities::TestContext& aTestContext)
{
    constexpr auto tRoundFunction = std::string_view{"my_round"};
    const auto tExpected = std::lround(kDoubleValue);
    EXPECT_EQ(aSharedLibrary.call<long(double)>(tRoundFunction, kDoubleValue), tExpected) << aTestContext;
}
}  // namespace

TEST(SharedLibrarySetupTeardown, ExceptionOnNoLibrary)
{
    constexpr auto tNonexistentSharedLibrary = std::string_view{"not-a-library.txt"};
    EXPECT_THROW([[maybe_unused]] auto tSharedLib = SharedLibrarySetupTeardown{tNonexistentSharedLibrary},
                 plato::utilities::Exception);
}

TEST(SharedLibrarySetupTeardown, ExceptionOnNonexistentFunction)
{
    auto tSharedLibrary = SharedLibrarySetupTeardown{kSharedLibraryName};
    constexpr auto tNonexistentFunction = std::string_view{"not_a_function"};
    EXPECT_THROW(tSharedLibrary.call<void()>(tNonexistentFunction), plato::utilities::Exception);
}

TEST(SharedLibrarySetupTeardown, LoadAndRunFunction)
{
    auto tSharedLibrary = SharedLibrarySetupTeardown{kSharedLibraryName};
    check_shared_lib_function(tSharedLibrary, TEST_CONTEXT("Load and run"));
}

TEST(SharedLibrarySetupTeardown, MoveCtor)
{
    auto tSharedLibraryToMove = SharedLibrarySetupTeardown{kSharedLibraryName};
    auto tSharedLibrary = SharedLibrarySetupTeardown{std::move(tSharedLibraryToMove)};

    check_shared_lib_function(tSharedLibrary, TEST_CONTEXT("Move ctor"));
}

TEST(SharedLibrarySetupTeardown, MoveAssignmentSameLibraries)
{
    auto tSharedLibraryToMove = SharedLibrarySetupTeardown{kSharedLibraryName};
    auto tSharedLibrary = SharedLibrarySetupTeardown{kSharedLibraryName};
    tSharedLibrary = std::move(tSharedLibraryToMove);

    check_shared_lib_function(tSharedLibrary, TEST_CONTEXT("Move assignment same libraries"));
}

TEST(SharedLibrarySetupTeardown, MoveAssignmentDifferentLibraries)
{
    auto tSharedLibraryToMove = SharedLibrarySetupTeardown{kSharedLibraryName};
    auto tSharedLibrary = SharedLibrarySetupTeardown{kBogusSharedLibraryName};
    tSharedLibrary = std::move(tSharedLibraryToMove);

    check_shared_lib_function(tSharedLibrary, TEST_CONTEXT("Move assignment different libraries"));
}

TEST_F(SharedLibraryCoutRedirectFixture, LibraryLoadFunction)
{
    auto tSharedLibraryToMove = SharedLibrarySetupTeardown{SharedLibraryData{
        .mSharedLibraryPath = kSharedLibraryName, .mLibraryLoadFunctionName = kLibraryLoadFunctionName}};

    const auto tExpectedOutput = std::string{"Test library was loaded.\n"};
    checkRankZeroCoutStringStream(tExpectedOutput, TEST_CONTEXT("Library load"));
}

TEST_F(SharedLibraryCoutRedirectFixture, MoveCtorLoadUnload)
{
    {
        auto tSharedLibraryToMove =
            SharedLibrarySetupTeardown{SharedLibraryData{.mSharedLibraryPath = kSharedLibraryName,
                                                         .mLibraryLoadFunctionName = kLibraryLoadFunctionName,
                                                         .mLibraryShutdownFunctionName = kLibraryUnloadFunctionName}};
        auto tSharedLibrary = SharedLibrarySetupTeardown{std::move(tSharedLibraryToMove)};
    }
    const auto tExpectedOutput = std::string{"Test library was loaded.\nTest library was unloaded.\n"};
    checkRankZeroCoutStringStream(tExpectedOutput, TEST_CONTEXT("Library unload"));
}

TEST_F(SharedLibraryCoutRedirectFixture, MoveAssignmentLoadUnload)
{
    {
        auto tSharedLibraryToMove =
            SharedLibrarySetupTeardown{SharedLibraryData{.mSharedLibraryPath = kSharedLibraryName,
                                                         .mLibraryLoadFunctionName = kLibraryLoadFunctionName,
                                                         .mLibraryShutdownFunctionName = kLibraryUnloadFunctionName}};
        auto tSharedLibrary =
            SharedLibrarySetupTeardown{SharedLibraryData{.mSharedLibraryPath = kBogusSharedLibraryName,
                                                         .mLibraryLoadFunctionName = std::nullopt,
                                                         .mLibraryShutdownFunctionName = kLibraryUnloadFunctionName}};
        tSharedLibrary = std::move(tSharedLibraryToMove);
    }
    const auto tExpectedOutput =
        std::string{"Test library was loaded.\nTest library was unloaded.\nBogus library unloading.\n"};
    checkRankZeroCoutStringStream(tExpectedOutput, TEST_CONTEXT("Library unload"));
}

TEST_F(SharedLibraryCoutRedirectFixture, LibraryUnloadFunction)
{
    {
        auto tSharedLibraryToMove =
            SharedLibrarySetupTeardown{SharedLibraryData{.mSharedLibraryPath = kSharedLibraryName,
                                                         .mLibraryLoadFunctionName = std::nullopt,
                                                         .mLibraryShutdownFunctionName = kLibraryUnloadFunctionName}};
    }
    const auto tExpectedOutput = std::string{"Test library was unloaded.\n"};
    checkRankZeroCoutStringStream(tExpectedOutput, TEST_CONTEXT("Library unload"));
}

TEST(SharedLibrarySetupTeardown, NonexistentLoadUnloadFunctionsThrows)
{
    {
        const auto tSharedLibraryData =
            SharedLibraryData{.mSharedLibraryPath = kSharedLibraryName, .mLibraryLoadFunctionName = "Nonexistent"};
        EXPECT_THROW([[maybe_unused]] auto tSharedLib = SharedLibrarySetupTeardown{tSharedLibraryData},
                     plato::utilities::Exception);
    }

    {
        const auto tSharedLibraryData = SharedLibraryData{.mSharedLibraryPath = kSharedLibraryName,
                                                          .mLibraryLoadFunctionName = std::nullopt,
                                                          .mLibraryShutdownFunctionName = "Nonexistent"};
        EXPECT_THROW([[maybe_unused]] auto tSharedLib = SharedLibrarySetupTeardown{tSharedLibraryData},
                     plato::utilities::Exception);
    }
}

}  // namespace plato::services::unittest
