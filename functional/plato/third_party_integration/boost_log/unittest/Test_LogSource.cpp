#include <gtest/gtest.h>

#include <sstream>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"

namespace plato::third_party_integration::boost_log::unittest
{
TEST(LogSource, StreamInsertion)
{
    const auto tExpectStreamMatches = [](const LogSource aLogSource, const std::string_view aExpected,
                                         const test_utilities::TestContext& aTestContext)
    {
        auto tStream = std::stringstream{};
        tStream << aLogSource;
        EXPECT_EQ(tStream.view(), aExpected) << aTestContext;
    };

    tExpectStreamMatches(LogSource::kInternal, "internal", TEST_CONTEXT("Internal"));
    tExpectStreamMatches(LogSource::kExternal, "external", TEST_CONTEXT("External"));
}
}  // namespace plato::third_party_integration::boost_log::unittest
