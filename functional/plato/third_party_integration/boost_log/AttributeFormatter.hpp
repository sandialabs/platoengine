#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_ATTRIBUTEFORMATTER
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_ATTRIBUTEFORMATTER

#include <boost/log/expressions/formatter.hpp>

#include "plato/third_party_integration/boost_log/AttributeTypes.hpp"

namespace plato::third_party_integration::boost_log
{
/// @brief Creates a formatter combining all arguments into a single formatter.
template <AttributeWithFormatter... Formatters>
[[nodiscard]] auto attribute_formatter() -> boost::log::formatter;

template <AttributeWithFormatter... Formatters>
auto attribute_formatter() -> boost::log::formatter
{
    using StreamType = boost::log::formatter::stream_type;

    return boost::log::formatter{[](const boost::log::record_view& aRecord, StreamType& aStream)
                                 { (Formatters::formatter()(aRecord, aStream), ...); }};
}

}  // namespace plato::third_party_integration::boost_log

#endif
