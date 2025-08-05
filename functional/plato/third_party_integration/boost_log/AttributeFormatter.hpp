#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_ATTRIBUTEFORMATTER
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_ATTRIBUTEFORMATTER

#include <boost/log/expressions/formatter.hpp>

#include "plato/third_party_integration/boost_log/AttributeTypes.hpp"

namespace plato::third_party_integration::boost_log
{
/// @brief Creates a formatter combining all template arguments into a single formatter.
/// @note Order matters, the attributes will be output from left to right matching the order of the formatters in the
/// template arguments.
template <Attribute... Formatters>
[[nodiscard]] auto attribute_formatter(FormattingStyle aFormattingStyle) -> boost::log::formatter;

template <Attribute... Formatters>
auto attribute_formatter(const FormattingStyle aFormattingStyle) -> boost::log::formatter
{
    using StreamType = boost::log::formatter::stream_type;

    const auto tFormatter =
        [aFormattingStyle]<typename AttributeType>(const boost::log::record_view& aRecord, StreamType& aStream)
    {
        if constexpr (AttributeWithFormatter<AttributeType>)
        {
            AttributeType::formatter(aFormattingStyle)(aRecord, aStream);
        }
    };

    return boost::log::formatter{[tFormatter](const boost::log::record_view& aRecord, StreamType& aStream)
                                 { (tFormatter.template operator()<Formatters>(aRecord, aStream), ...); }};
}

}  // namespace plato::third_party_integration::boost_log

#endif
