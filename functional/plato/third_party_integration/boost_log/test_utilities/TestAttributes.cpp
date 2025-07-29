#include "plato/third_party_integration/boost_log/test_utilities/TestAttributes.hpp"

#include <boost/log/expressions.hpp>

namespace plato::third_party_integration::boost_log::test_utilities
{
auto SharkAttribute::formatter() -> boost::log::formatter
{
    namespace ble = boost::log::expressions;
    return boost::log::formatter{
        ble::stream << ble::if_(ble::has_attr(shark_attribute))[ble::stream << "{" << shark_attribute << "} "]};
}

auto SharkAttribute::filter() -> boost::log::filter
{
    return boost::log::filter{boost::log::expressions::has_attr(shark_attribute) &&
                              (shark_attribute == "hammerhead" || shark_attribute == "great-white")};
}
}  // namespace plato::third_party_integration::boost_log::test_utilities
