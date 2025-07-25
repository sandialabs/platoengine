#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_COMPONENTATTRIBUTE
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_COMPONENTATTRIBUTE

#include <boost/log/expressions/keyword.hpp>
#include <string_view>

#include "plato/components/ComponentType.hpp"

namespace plato::third_party_integration::boost_log
{
constexpr inline auto kComponentTypeAttributeName = std::string_view{"Component"};
}

BOOST_LOG_ATTRIBUTE_KEYWORD(component_attribute,
                            plato::third_party_integration::boost_log::kComponentTypeAttributeName.data(),
                            plato::components::ComponentType)

#endif
