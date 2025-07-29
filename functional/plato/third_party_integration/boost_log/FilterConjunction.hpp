#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_FILTERCONJUNCTION
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_FILTERCONJUNCTION

#include <boost/log/expressions/filter.hpp>

#include "plato/third_party_integration/boost_log/AttributeTypes.hpp"

namespace plato::third_party_integration::boost_log
{
/// @brief Returns a filter representing the conjunction (logical `and` operation) of all attribute filters in the
/// template arguments.
template <Attribute... Filters>
[[nodiscard]] auto filter_conjunction() -> boost::log::filter;

template <Attribute... Filters>
auto filter_conjunction() -> boost::log::filter
{
    return boost::log::filter{[](const boost::log::attribute_value_set& aAttributes)
                              {
                                  const auto tFilter =
                                      []<typename AttributeType>(const boost::log::attribute_value_set& aAttributes)
                                  {
                                      if constexpr (AttributeWithFilter<AttributeType>)
                                      {
                                          return AttributeType::filter()(aAttributes);
                                      }
                                      return true;
                                  };
                                  return (tFilter.template operator()<Filters>(aAttributes) && ...);
                              }};
}
}  // namespace plato::third_party_integration::boost_log

#endif
