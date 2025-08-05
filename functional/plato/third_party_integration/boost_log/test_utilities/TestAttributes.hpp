#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_TESTATTRIBUTES
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_TESTATTRIBUTES

#include <boost/log/expressions/filter.hpp>
#include <boost/log/expressions/formatter.hpp>
#include <string>
#include <string_view>

#include "plato/third_party_integration/boost_log/AttributeTypes.hpp"

namespace plato::third_party_integration::boost_log::test_utilities
{
struct SharkAttribute
{
    using AttributeType = std::string;
    AttributeType mValue;

    [[nodiscard]] constexpr static auto name() -> std::string_view { return "Sharks!"; }
    [[nodiscard]] static auto formatter(FormattingStyle aFormattingStyle) -> boost::log::formatter;
    [[nodiscard]] static auto filter() -> boost::log::filter;
};

}  // namespace plato::third_party_integration::boost_log::test_utilities

BOOST_LOG_ATTRIBUTE_KEYWORD(
    shark_attribute,
    plato::third_party_integration::boost_log::test_utilities::SharkAttribute::name().data(),
    typename plato::third_party_integration::boost_log::test_utilities::SharkAttribute::AttributeType)

#endif
