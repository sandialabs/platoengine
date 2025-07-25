#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_TEST_UTILITIES_TESTUTILITIES
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_TEST_UTILITIES_TESTUTILITIES

#include <boost/log/attributes.hpp>
#include <boost/log/attributes/attribute_set.hpp>
#include <boost/log/attributes/attribute_value_set.hpp>
#include <string_view>
#include <type_traits>

namespace plato::third_party_integration::boost_log::test_utilities
{
/// @brief Returns an attribute_set containing a single entry given by @a aAttribute name and @a aAttributeValue.
template <typename T>
[[nodiscard]] auto attribute_set(std::string_view aAttributeName, T&& aAttributeValue) -> boost::log::attribute_set;

/// @brief Returns an attribute_value_set containing @a aAttributeSet as the source attributes.
[[nodiscard]] auto attribute_value_set(const boost::log::attribute_set& aAttributeSet)
    -> boost::log::attribute_value_set;

template <typename T>
auto attribute_set(std::string_view aAttributeName, T&& aAttributeValue) -> boost::log::attribute_set
{
    auto tAttributeSet = boost::log::attribute_set{};
    tAttributeSet.insert(boost::log::attribute_name{std::string{aAttributeName}},
                         boost::log::attributes::constant<std::decay_t<T>>(std::forward<T>(aAttributeValue)));
    return tAttributeSet;
}
}  // namespace plato::third_party_integration::boost_log::test_utilities

#endif
