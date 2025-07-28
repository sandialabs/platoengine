#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_COMPONENTATTRIBUTES
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_COMPONENTATTRIBUTES

#include <boost/log/expressions/formatter.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>

#include "plato/components/ComponentType.hpp"
#include "plato/third_party_integration/boost_log/AttributeTypes.hpp"

namespace plato::third_party_integration::boost_log
{
/// @brief Stores the type and name of a component, such as level-set geometry
struct ComponentTypeAndName
{
    plato::components::ComponentType mComponentType;
    std::string mComponentName;
};

/// @brief Stream insertion operator for a component type and name. Inserts a colon in between the type and name.
auto operator<<(std::ostream& aStream, const ComponentTypeAndName& aComponentTypeAndName) -> std::ostream&;

/// @brief Attribute for specifying the MPI rank of a log message.
struct ComponentTypeAndNameAttribute
{
    using AttributeType = ComponentTypeAndName;
    AttributeType mValue;

    constexpr static inline auto name() -> std::string_view { return std::string_view{"Component"}; }

    [[nodiscard]] static auto formatter() -> boost::log::formatter;
};

static_assert(AttributeWithFormatter<ComponentTypeAndNameAttribute>,
              "ComponentTypeAndNameAttribute satisfies concept AttributeWithFormatter");

}  // namespace plato::third_party_integration::boost_log

BOOST_LOG_ATTRIBUTE_KEYWORD(component_attribute,
                            plato::third_party_integration::boost_log::ComponentTypeAndNameAttribute::name().data(),
                            plato::third_party_integration::boost_log::ComponentTypeAndName)

#endif
