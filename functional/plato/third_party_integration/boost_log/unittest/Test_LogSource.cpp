#include <gtest/gtest.h>

#include <boost/log/attributes/constant.hpp>
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

TEST(LogSource, LogSourceFilter)
{
    const auto tLogSourceFilter = log_source_filter(LogSource::kInternal);

    auto tAttributeSet = boost::log::attribute_set{};
    tAttributeSet.insert(boost::log::attribute_name{kLogSourceAttributeName.data()},
                         boost::log::attributes::constant<LogSource>(LogSource::kInternal));

    EXPECT_TRUE(tLogSourceFilter(
        boost::log::attribute_value_set{tAttributeSet, boost::log::attribute_set{}, boost::log::attribute_set{}}));
}

}  // namespace plato::third_party_integration::boost_log::unittest
