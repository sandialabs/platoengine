#include <gtest/gtest.h>

#include <boost/log/attributes/constant.hpp>
#include <sstream>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/test_utilities/TestUtilities.hpp"

namespace plato::third_party_integration::boost_log::unittest
{
TEST(LogSource, StreamInsertion)
{
    const auto tExpectStreamMatches = [](const LogSource aLogSource, const std::string_view aExpected,
                                         const plato::test_utilities::TestContext& aTestContext)
    {
        auto tStream = std::stringstream{};
        tStream << aLogSource;
        EXPECT_EQ(tStream.view(), aExpected) << aTestContext;
    };

    tExpectStreamMatches(LogSource::kInternal, "internal", TEST_CONTEXT("Internal"));
    tExpectStreamMatches(LogSource::kExternal, "external", TEST_CONTEXT("External"));
}

TEST(LogSource, LogSourceFilter)
{
    const auto tLogSourceFilter = log_source_filter(LogSource::kInternal);
    const auto tLogSourceAttribute = test_utilities::attribute_set(kLogSourceAttributeName, LogSource::kInternal);
    const auto tLogSourceAttributeSet = test_utilities::attribute_value_set(tLogSourceAttribute);
    EXPECT_TRUE(tLogSourceFilter(tLogSourceAttributeSet));
}

}  // namespace plato::third_party_integration::boost_log::unittest
