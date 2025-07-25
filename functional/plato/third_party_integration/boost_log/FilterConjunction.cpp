#include "plato/third_party_integration/boost_log/FilterConjunction.hpp"

namespace plato::third_party_integration::boost_log
{
auto filter_conjunction(boost::log::filter aLeftFilter, boost::log::filter aRightFilter) -> boost::log::filter
{
    return boost::log::filter{[mLeftFilter = std::move(aLeftFilter), mRightFilter = std::move(aRightFilter)](
                                  const boost::log::attribute_value_set& aAttributes)
                              { return mLeftFilter(aAttributes) && mRightFilter(aAttributes); }};
}
}  // namespace plato::third_party_integration::boost_log
