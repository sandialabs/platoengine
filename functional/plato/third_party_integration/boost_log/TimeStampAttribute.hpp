#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_TIMESTAMPATTRIBUTE
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_TIMESTAMPATTRIBUTE

#include <boost/log/attributes/clock.hpp>
#include <boost/log/expressions/formatter.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <string_view>

#include "plato/third_party_integration/boost_log/AttributeTypes.hpp"

namespace plato::third_party_integration::boost_log
{
/// @brief Attribute for a time stamp, with formatting given by `[YY-MM-DD HH:MM:SS]`
struct TimeStampAttribute
{
    using AttributeType = boost::log::attributes::local_clock;
    AttributeType mValue;

    [[nodiscard]] constexpr static auto name() -> std::string_view;

    [[nodiscard]] static auto formatter() -> boost::log::formatter;
};

constexpr auto TimeStampAttribute::name() -> std::string_view { return std::string_view{"Time stamp"}; }

static_assert(AttributeWithFormatter<TimeStampAttribute>,
              "TimeStampAttribute must satisfy concept AttributeWithFormatter");

}  // namespace plato::third_party_integration::boost_log

BOOST_LOG_ATTRIBUTE_KEYWORD(time_stamp_attribute,
                            plato::third_party_integration::boost_log::TimeStampAttribute::name().data(),
                            typename plato::third_party_integration::boost_log::TimeStampAttribute::AttributeType);

#endif
