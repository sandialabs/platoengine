#include <gtest/gtest.h>

#include <boost/log/attributes/constant.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/boost_log/AttributeFormatter.hpp"
#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"

namespace plato::third_party_integration::boost_log::unittest
{
namespace
{
constexpr auto kColorAttribute = std::string_view{"color"};
constexpr auto kSizeAttribute = std::string_view{"size"};
}  // namespace

BOOST_LOG_ATTRIBUTE_KEYWORD(color_attribute,
                            plato::third_party_integration::boost_log::unittest::kColorAttribute.data(),
                            std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(size_attribute,
                            plato::third_party_integration::boost_log::unittest::kSizeAttribute.data(),
                            std::string)
}  // namespace plato::third_party_integration::boost_log::unittest

namespace plato::third_party_integration::boost_log::unittest
{
namespace
{
struct ColorAttribute
{
    using AttributeType = std::string;
    AttributeType mValue;

    [[nodiscard]] constexpr static auto name() -> std::string_view { return kColorAttribute; }

    [[nodiscard]] static auto formatter(const FormattingStyle aFormattingStyle) -> boost::log::formatter
    {
        namespace ble = boost::log::expressions;
        if (aFormattingStyle == FormattingStyle::kColor)
        {
            return boost::log::formatter{
                ble::stream << ble::if_(ble::has_attr(color_attribute))[ble::stream << "<" << color_attribute << ">"]};
        }
        return boost::log::formatter{[](const boost::log::record_view&, boost::log::formatter::stream_type&) {}};
    }
};

struct SizeAttribute
{
    using AttributeType = std::string;
    AttributeType mValue;

    [[nodiscard]] constexpr static auto name() -> std::string_view { return kSizeAttribute; }

    [[nodiscard]] static auto formatter(FormattingStyle) -> boost::log::formatter
    {
        namespace ble = boost::log::expressions;
        return boost::log::formatter{
            ble::stream << ble::if_(ble::has_attr(size_attribute))[ble::stream << "{" << size_attribute << "}"]};
    }
};
}  // namespace

TEST(AttributeFormatter, AttributeFormatter)
{
    const auto tCheckFormatting = [](const FormattingStyle aFormattingStyle, const std::string_view aExpected,
                                     const test_utilities::TestContext& aTestContext)
    {
        const auto tFormatter = attribute_formatter<ColorAttribute, SizeAttribute>(aFormattingStyle);

        const auto tStream = boost::make_shared<std::stringstream>();
        [[maybe_unused]] const auto tInternalLoggerSink =
            LoggerSinkSetupTeardown{tStream, tFormatter, boost::log::filter{}};

        auto tLogger = boost::log::sources::logger{};
        tLogger.add_attribute(kColorAttribute.data(), boost::log::attributes::make_constant(std::string{"red"}));
        tLogger.add_attribute(kSizeAttribute.data(), boost::log::attributes::make_constant(std::string{"little"}));

        BOOST_LOG(tLogger) << "should not appear";

        EXPECT_EQ(aExpected, tStream->str()) << aTestContext;
    };

    constexpr auto tExpectedNoColor = std::string_view{"{little}\n"};
    tCheckFormatting(FormattingStyle::kNone, tExpectedNoColor, TEST_CONTEXT("No color"));

    constexpr auto tExpectedColor = std::string_view{"<red>{little}\n"};
    tCheckFormatting(FormattingStyle::kColor, tExpectedColor, TEST_CONTEXT("Color"));
}
}  // namespace plato::third_party_integration::boost_log::unittest
