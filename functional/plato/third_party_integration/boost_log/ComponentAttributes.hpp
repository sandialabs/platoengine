#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_COMPONENTATTRIBUTES
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_COMPONENTATTRIBUTES

#include <boost/log/expressions/formatter.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>

#include "plato/components/ComponentType.hpp"

namespace plato::third_party_integration::boost_log
{
/// @brief Stores the type and name of a component, such as level-set geometry
struct ComponentTypeAndName
{
    plato::components::ComponentType mComponentType;
    std::string mComponentName;
};

constexpr inline auto kComponentAttributeName = std::string_view{"Component"};

/// @brief Returns a boost formatter that formats the component type and name for log output
[[nodiscard]] auto component_attributes_formatter() -> boost::log::formatter;

/// @brief Stream insertion operator for a component type and name. Inserts a colon in between the type and name.
auto operator<<(std::ostream& aStream, const ComponentTypeAndName& aComponentTypeAndName) -> std::ostream&;

}  // namespace plato::third_party_integration::boost_log

BOOST_LOG_ATTRIBUTE_KEYWORD(component_attribute,
                            plato::third_party_integration::boost_log::kComponentAttributeName.data(),
                            plato::third_party_integration::boost_log::ComponentTypeAndName)

#endif
