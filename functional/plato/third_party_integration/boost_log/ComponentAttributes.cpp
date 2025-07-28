#include "plato/third_party_integration/boost_log/ComponentAttributes.hpp"

#include <boost/log/expressions.hpp>

#include "plato/components/ComponentTypeStream.hpp"

namespace plato::third_party_integration::boost_log
{
[[nodiscard]] auto ComponentTypeAndNameAttribute::formatter() -> boost::log::formatter
{
    namespace ble = boost::log::expressions;
    return boost::log::formatter{
        ble::stream << ble::if_(ble::has_attr(component_attribute))[ble::stream << "[" << component_attribute << "] "]};
}

auto operator<<(std::ostream& aStream, const ComponentTypeAndName& aComponentTypeAndName) -> std::ostream&
{
    aStream << aComponentTypeAndName.mComponentType << ":" << aComponentTypeAndName.mComponentName;
    return aStream;
}
}  // namespace plato::third_party_integration::boost_log
