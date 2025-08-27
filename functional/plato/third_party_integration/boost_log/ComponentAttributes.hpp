#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_COMPONENTATTRIBUTES
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_COMPONENTATTRIBUTES

#include <boost/log/expressions.hpp>
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

/// @brief Attribute for specifying the component type and name of a message.
struct ComponentTypeAndNameAttribute
{
    using AttributeType = ComponentTypeAndName;
    AttributeType mValue;

    [[nodiscard]] constexpr static inline auto name() -> std::string_view;

    [[nodiscard]] static auto formatter(FormattingStyle aFormattingStyle) -> boost::log::formatter;
};

/// @brief Attribute for filtering messages only from a specific component.
template <components::ComponentType kIncludedComponent>
struct ComponentTypeFilterAttribute
{
    using AttributeType = components::ComponentType;
    AttributeType mValue = kIncludedComponent;

    [[nodiscard]] constexpr static inline auto name() -> std::string_view;

    [[nodiscard]] static auto filter() -> boost::log::filter;
};

/// @brief Stream insertion operator for a component type and name. Inserts a colon in between the type and name.
auto operator<<(std::ostream& aStream, const ComponentTypeAndName& aComponentTypeAndName) -> std::ostream&;

constexpr inline auto ComponentTypeAndNameAttribute::name() -> std::string_view
{
    return std::string_view{"Component"};
}

template <components::ComponentType kIncludedComponent>
constexpr inline auto ComponentTypeFilterAttribute<kIncludedComponent>::name() -> std::string_view
{
    return std::string_view{"Component type"};
}

static_assert(AttributeWithFormatter<ComponentTypeAndNameAttribute>,
              "ComponentTypeAndNameAttribute must satisfy concept AttributeWithFormatter");

static_assert(AttributeWithFilter<ComponentTypeFilterAttribute<components::ComponentType::kFilter>>,
              "ComponentTypeFilterAttribute must satisfy concept AttributeWithFilter");

}  // namespace plato::third_party_integration::boost_log

BOOST_LOG_ATTRIBUTE_KEYWORD(component_attribute,
                            plato::third_party_integration::boost_log::ComponentTypeAndNameAttribute::name().data(),
                            plato::third_party_integration::boost_log::ComponentTypeAndName)

BOOST_LOG_ATTRIBUTE_KEYWORD(component_filter_attribute,
                            plato::third_party_integration::boost_log::ComponentTypeFilterAttribute<
                                plato::components::ComponentType::kFilter>::name()
                                .data(),
                            plato::components::ComponentType)

namespace plato::third_party_integration::boost_log
{
template <components::ComponentType kIncludedComponent>
inline auto ComponentTypeFilterAttribute<kIncludedComponent>::filter() -> boost::log::filter
{
    return boost::log::filter{boost::log::expressions::has_attr(component_filter_attribute) &&
                              component_filter_attribute == kIncludedComponent};
}
}  // namespace plato::third_party_integration::boost_log

#endif
