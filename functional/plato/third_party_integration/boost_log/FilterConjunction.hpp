#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_COMPOSEFILTER
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_COMPOSEFILTER

#include <boost/log/expressions/filter.hpp>

namespace plato::third_party_integration::boost_log
{
/// @brief Returns a filter representing the conjunction (logical `and` operation) of @a aLeftFilter and @a aRightFilter
[[nodiscard]] auto filter_conjunction(boost::log::filter aLeftFilter,
                                      boost::log::filter aRightFilter) -> boost::log::filter;
}  // namespace plato::third_party_integration::boost_log

#endif
