#include <gtest/gtest.h>

#include <boost/log/attributes/constant.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include "plato/test_utilities/CoutCerrPrintTestFixture.hpp"
#include "plato/third_party_integration/boost_log/ComponentAttributes.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"
#include "plato/third_party_integration/boost_log/SinkWithAttributeFormattersAndFilters.hpp"
#include "plato/third_party_integration/boost_log/test_utilities/TestAttributes.hpp"

namespace plato::third_party_integration::boost_log::unittest
{
namespace
{
constexpr auto kSharkTag = std::string_view{"great-white"};

class SinkWithAttributeFormattersAndFiltersCoutRedirect : public plato::test_utilities::CoutCerrPrintTestFixture
{
};

[[nodiscard]] auto test_logger() -> boost::log::sources::logger
{
    auto tLogger = boost::log::sources::logger{};
    tLogger.add_attribute(test_utilities::SharkAttribute::name().data(),
                          boost::log::attributes::make_constant(std::string{kSharkTag}));
    return tLogger;
}
}  // namespace

TEST(SinkWithAttributeFormattersAndFilters, Outputs)
{
    const auto tStream = boost::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLogSink =
        sink_with_attribute_formatters_and_filters<test_utilities::SharkAttribute>(tStream);

    constexpr auto tMessage = std::string_view{"It lives in a string!"};
    auto tLogger = test_logger();
    BOOST_LOG(tLogger) << tMessage;

    EXPECT_NE(tStream->str().find(tMessage), std::string::npos) << "Result: " << tStream->str();
    EXPECT_NE(tStream->str().find(kSharkTag), std::string::npos) << "Result: " << tStream->str();
}

TEST_F(SinkWithAttributeFormattersAndFiltersCoutRedirect, Outputs)
{
    [[maybe_unused]] const auto tLogSink = sink_with_attribute_formatters_and_filters<test_utilities::SharkAttribute>();

    constexpr auto tMessage = std::string_view{"It lives in cout!"};
    auto tLogger = test_logger();
    BOOST_LOG(tLogger) << tMessage;

    checkRankZeroCoutStringStreamStreamForPattern({std::string{tMessage}, std::string{kSharkTag}},
                                                  TEST_CONTEXT("Checking for log message."));
}
}  // namespace plato::third_party_integration::boost_log::unittest
