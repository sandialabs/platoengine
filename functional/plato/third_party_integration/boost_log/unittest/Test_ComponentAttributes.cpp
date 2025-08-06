#include <gtest/gtest.h>

#include <boost/log/attributes/constant.hpp>
#include <boost/log/expressions/formatters/stream.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/boost_log/ComponentAttributes.hpp"
#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"
#include "plato/third_party_integration/boost_log/test_utilities/TestUtilities.hpp"
#include "plato/utilities/Colorize.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::third_party_integration::boost_log::unttest
{
TEST(ComponentAttributes, Formatter)
{
    const auto tCheckFormatting = [](const FormattingStyle aFormattingStyle, const std::string_view aExpected,
                                     const plato::test_utilities::TestContext& aTestContext)
    {
        const auto tStream = boost::make_shared<std::stringstream>();

        const auto tFormatter = ComponentTypeAndNameAttribute::formatter(aFormattingStyle);
        [[maybe_unused]] const auto tInternalLoggerSink =
            LoggerSinkSetupTeardown{tStream, tFormatter, boost::log::filter{}};

        auto tLogger = boost::log::sources::logger{};
        tLogger.add_attribute(
            ComponentTypeAndNameAttribute::name().data(),
            boost::log::attributes::make_constant(ComponentTypeAndName{
                .mComponentType = components::ComponentType::kFilter, .mComponentName = "helmholtz"}));

        BOOST_LOG(tLogger) << "this message should not appear";

        EXPECT_EQ(tStream->str(), aExpected) << aTestContext;
    };

    constexpr auto tExpectedNoColor = std::string_view{"[filter:helmholtz] \n"};
    tCheckFormatting(FormattingStyle::kNone, tExpectedNoColor, TEST_CONTEXT("No color"));

    const auto tExpectedColor =
        utilities::concatenate("[", utilities::color_code(utilities::TextColor::kDarkGray), "filter:helmholtz",
                               utilities::color_code(utilities::TextColor::kDefault), "] \n");
    tCheckFormatting(FormattingStyle::kColor, tExpectedColor, TEST_CONTEXT("Color"));
}

TEST(ComponentAttributes, StreamInsertion)
{
    auto tStream = std::stringstream{};
    tStream << ComponentTypeAndName{.mComponentType = components::ComponentType::kGeometry,
                                    .mComponentName = "mobius-strip"};
    EXPECT_EQ(tStream.str(), "geometry:mobius-strip");
}

TEST(ComponentAttributes, ComponentTypeFilter)
{
    constexpr auto tObjectiveComponent = components::ComponentType::kObjective;
    const auto tComponentTypeFilter = ComponentTypeFilterAttribute<tObjectiveComponent>::filter();

    {
        const auto tComponentAttribute = test_utilities::attribute_set(
            ComponentTypeFilterAttribute<tObjectiveComponent>::name(), tObjectiveComponent);
        const auto tAttributeValueSet = test_utilities::attribute_value_set(tComponentAttribute);
        EXPECT_TRUE(tComponentTypeFilter(tAttributeValueSet));
    }
    {
        const auto tComponentAttribute = test_utilities::attribute_set(
            ComponentTypeFilterAttribute<tObjectiveComponent>::name(), components::ComponentType::kFilter);
        const auto tAttributeValueSet = test_utilities::attribute_value_set(tComponentAttribute);
        EXPECT_FALSE(tComponentTypeFilter(tAttributeValueSet));
    }
}

}  // namespace plato::third_party_integration::boost_log::unttest
