#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_ATTRIBUTEFORMATTER
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_ATTRIBUTEFORMATTER

#include <boost/log/expressions/formatter.hpp>

namespace plato::third_party_integration::boost_log
{
/// @brief Creates a formatter combining all arguments into a single formatter.
template <typename... Formatter>
    requires std::conjunction_v<std::is_same<Formatter, boost::log::formatter>...>
[[nodiscard]] auto attribute_formatter(Formatter... aAttributeFormatters) -> boost::log::formatter;

template <typename... Formatter>
    requires std::conjunction_v<std::is_same<Formatter, boost::log::formatter>...>
auto attribute_formatter(Formatter... aAttributeFormatters) -> boost::log::formatter
{
    using StreamType = boost::log::formatter::stream_type;

    return boost::log::formatter{
        [... mFormatters = std::move(aAttributeFormatters)](const boost::log::record_view& aRecord, StreamType& aStream)
        { (mFormatters(aRecord, aStream), ...); }};
}

}  // namespace plato::third_party_integration::boost_log

#endif
