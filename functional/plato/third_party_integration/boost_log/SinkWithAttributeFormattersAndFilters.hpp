#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_SINKWITHATTRIBUTEFORMATTERSANDFILTERS
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_SINKWITHATTRIBUTEFORMATTERSANDFILTERS

#include <boost/core/null_deleter.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/formatters/stream.hpp>
#include <iostream>
#include <ostream>

#include "plato/third_party_integration/boost_log/AttributeFormatter.hpp"
#include "plato/third_party_integration/boost_log/AttributeTypes.hpp"
#include "plato/third_party_integration/boost_log/FilterConjunction.hpp"
#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"

namespace plato::third_party_integration::boost_log
{
namespace detail
{
template <typename Attribute>
concept AttributeWithFormatterOrFilter = AttributeWithFormatter<Attribute> || AttributeWithFilter<Attribute>;
}

/// @brief Returns a sink that streams to @a aStream and sets up filters and formatters.
///
/// The template parameters must satisfy the either concept AttributeWithFormatter or AttributeWithFilter.
/// All attributes with filters will be used as filters and all attributes with formatters will be used as formatters.
template <detail::AttributeWithFormatterOrFilter... Attributes>
[[nodiscard]] auto sink_with_attribute_formatters_and_filters(const boost::shared_ptr<std::ostream>& aStreamSink)
    -> LoggerSinkSetupTeardown;

/// @brief This overload returns a sink that streams to `std::cout`.
///
/// The template parameters must satisfy the either concept AttributeWithFormatter or AttributeWithFilter.
/// All attributes with filters will be used as filters and all attributes with formatters will be used as formatters.
template <detail::AttributeWithFormatterOrFilter... Attributes>
[[nodiscard]] auto sink_with_attribute_formatters_and_filters() -> LoggerSinkSetupTeardown;

template <detail::AttributeWithFormatterOrFilter... Attributes>
auto sink_with_attribute_formatters_and_filters(const boost::shared_ptr<std::ostream>& aStreamSink)
    -> LoggerSinkSetupTeardown
{
    auto tAttributeFormatter = attribute_formatter<Attributes...>();
    auto tMessageFormatter =
        boost::log::formatter{boost::log::expressions::stream << boost::log::expressions::smessage};

    auto tFormatter =
        [mAttributeFormatter = std::move(tAttributeFormatter), mMessageFormatter = std::move(tMessageFormatter)](
            const boost::log::record_view& aRecord, boost::log::formatter::stream_type& aStream)
    {
        mAttributeFormatter(aRecord, aStream);
        mMessageFormatter(aRecord, aStream);
    };

    return LoggerSinkSetupTeardown{aStreamSink, boost::log::formatter{std::move(tFormatter)},
                                   filter_conjunction<Attributes...>()};
}

template <detail::AttributeWithFormatterOrFilter... Attributes>
auto sink_with_attribute_formatters_and_filters() -> LoggerSinkSetupTeardown
{
    return sink_with_attribute_formatters_and_filters<Attributes...>(
        boost::shared_ptr<std::ostream>{&std::cout, boost::null_deleter()});
}

}  // namespace plato::third_party_integration::boost_log

#endif
