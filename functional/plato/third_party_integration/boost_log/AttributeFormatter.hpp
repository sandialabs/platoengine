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
[[nodiscard]] auto attribute_formatter() -> boost::log::formatter;

template <Attribute... Formatters>
auto attribute_formatter() -> boost::log::formatter
{
    using StreamType = boost::log::formatter::stream_type;

    return boost::log::formatter{[](const boost::log::record_view& aRecord, StreamType& aStream)
                                 {
                                     const auto tFormatter =
                                         []<typename AttributeType>(const boost::log::record_view& aRecord,
                                                                    StreamType& aStream)
                                     {
                                         if constexpr (AttributeWithFormatter<AttributeType>)
                                         {
                                             AttributeType::formatter()(aRecord, aStream);
                                         }
                                     };
                                     (tFormatter.template operator()<Formatters>(aRecord, aStream), ...);
                                 }};
}

}  // namespace plato::third_party_integration::boost_log

#endif
