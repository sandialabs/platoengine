#include "plato/third_party_integration/boost_log/test_utilities/TestUtilities.hpp"

namespace plato::third_party_integration::boost_log::test_utilities
{
auto attribute_value_set(const boost::log::attribute_set& aAttributeSet) -> boost::log::attribute_value_set
{
    return boost::log::attribute_value_set{aAttributeSet, boost::log::attribute_set{}, boost::log::attribute_set{}};
}

}  // namespace plato::third_party_integration::boost_log::test_utilities
