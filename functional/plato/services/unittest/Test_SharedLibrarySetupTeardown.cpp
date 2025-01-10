#include <gtest/gtest.h>

#include <cmath>
#include <string_view>

#include "plato/services/SharedLibrarySetupTeardown.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::services::unittest
{
namespace
{
constexpr auto kSharedLibraryName = "libPlatoServicesTestSharedLib.so";
constexpr auto kBogusSharedLibraryName = "libPlatoServicesTestBogusSharedLib.so";
constexpr auto kDoubleValue = double{42.2};

void check_shared_lib_function(SharedLibrarySetupTeardown& aSharedLibrary,
                               const test_utilities::TestContext& aTestContext)
{
    constexpr auto tRoundFunction = std::string_view{"my_round"};
    const auto tExpected = std::lround(kDoubleValue);
    EXPECT_EQ(aSharedLibrary.call<long(double)>(tRoundFunction, kDoubleValue), tExpected) << aTestContext;
}
}

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

}  // namespace plato::services::unittest
