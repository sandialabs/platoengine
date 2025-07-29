#include <gtest/gtest.h>

#include <boost/log/attributes/constant.hpp>
#include <boost/log/expressions/formatters/stream.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include "plato/third_party_integration/boost_log/ComponentAttributes.hpp"
#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"

namespace plato::third_party_integration::boost_log::unttest
{
TEST(ComponentAttributes, Formatter)
{
    const auto tStream = boost::make_shared<std::stringstream>();

    const auto tFormatter = ComponentTypeAndNameAttribute::formatter();
    [[maybe_unused]] const auto tInternalLoggerSink =
        LoggerSinkSetupTeardown{tStream, tFormatter, boost::log::filter{}};

    auto tLogger = boost::log::sources::logger{};
    tLogger.add_attribute(ComponentTypeAndNameAttribute::name().data(),
                          boost::log::attributes::make_constant(ComponentTypeAndName{
                              .mComponentType = components::ComponentType::kFilter, .mComponentName = "helmholtz"}));

    BOOST_LOG(tLogger) << "this message should not appear";

    EXPECT_EQ(tStream->str(), "[filter:helmholtz] \n");
}

TEST(ComponentAttributes, StreamInsertion)
{
    auto tStream = std::stringstream{};
    tStream << ComponentTypeAndName{.mComponentType = components::ComponentType::kGeometry,
                                    .mComponentName = "mobius-strip"};
    EXPECT_EQ(tStream.str(), "geometry:mobius-strip");
}
}  // namespace plato::third_party_integration::boost_log::unttest
