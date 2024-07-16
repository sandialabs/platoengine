#include <gtest/gtest.h>

#include <cmath>
#include <string_view>

#include "plato/services/SharedLibrarySetupTeardown.hpp"

namespace plato::services::unittest
{
namespace
{
constexpr auto kSharedLibName = "libPlatoServicesTestSharedLib.so";
}

TEST(SharedLibrarySetupTeardown, ExceptionOnNoLibrary)
{
    constexpr auto tNonexistentSharedLib = std::string_view{"not-a-library.txt"};
    EXPECT_THROW([[maybe_unused]] auto tSharedLib = SharedLibrarySetupTeardown{tNonexistentSharedLib},
                 plato::utilities::Exception);
}

TEST(SharedLibrarySetupTeardown, ExceptionOnNonexistentFunction)
{
    auto tSharedLib = SharedLibrarySetupTeardown{kSharedLibName};
    constexpr auto tNonexistentFunction = std::string_view{"not_a_function"};
    EXPECT_THROW(tSharedLib.call<void()>(tNonexistentFunction), plato::utilities::Exception);
}

TEST(SharedLibrarySetupTeardown, LoadAndRunFunction)
{
    auto tSharedLib = SharedLibrarySetupTeardown{kSharedLibName};
    constexpr auto tRoundFunction = std::string_view{"my_round"};
    const auto tDoubleValue = double{42.2};
    const auto tExpected = std::lround(tDoubleValue);
    EXPECT_EQ(tSharedLib.call<long(double)>(tRoundFunction, tDoubleValue), tExpected);
}

}  // namespace plato::services::unittest
