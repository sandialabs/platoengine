#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_FILTERCONJUNCTION
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_FILTERCONJUNCTION

#include <boost/log/expressions/filter.hpp>

#include "plato/third_party_integration/boost_log/AttributeTypes.hpp"

namespace plato::third_party_integration::boost_log
{
/// @brief Returns a filter representing the conjunction (logical `and` operation) of all attribute filters in the
/// template arguments.
template <AttributeWithFilter... Attributes>
[[nodiscard]] auto filter_conjunction() -> boost::log::filter;

template <AttributeWithFilter... Attributes>
auto filter_conjunction() -> boost::log::filter
{
    return boost::log::filter{[](const boost::log::attribute_value_set& aAttributes)
                              { return (Attributes::filter()(aAttributes) && ...); }};
}
}  // namespace plato::third_party_integration::boost_log

#endif
