#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_ATTRIBUTETYPES
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_ATTRIBUTETYPES

#include <boost/log/expressions/filter.hpp>
#include <boost/log/expressions/formatter.hpp>
#include <concepts>
#include <cstdint>
#include <string_view>

namespace plato::third_party_integration::boost_log
{
/// @brief Formatting style, mainly indicating whether or not to use color.
enum struct FormattingStyle : std::uint8_t
{
    kNone,
    kColor
};

/// @brief Concept defining the requirements of an attribute type.
template <typename T>
concept Attribute = requires(const T t) {
    {
        T::name()
    } -> std::convertible_to<std::string_view>;
    {
        t.mValue
    } -> std::convertible_to<typename T::AttributeType>;
    typename T::AttributeType;
};

/// @brief Concept defining the requirements of an attribute type that provides a formatter.
template <typename T>
concept AttributeWithFormatter = requires(const T t) {
    {
        T::name()
    } -> std::convertible_to<std::string_view>;
    {
        T::formatter(FormattingStyle{})
    } -> std::convertible_to<boost::log::formatter>;
    {
        t.mValue
    } -> std::convertible_to<typename T::AttributeType>;
    typename T::AttributeType;
};

/// @brief Concept defining the requirements of an attribute type that provides a filter.
template <typename T>
concept AttributeWithFilter = requires(const T t) {
    {
        T::name()
    } -> std::convertible_to<std::string_view>;
    {
        T::filter()
    } -> std::convertible_to<boost::log::filter>;
    {
        t.mValue
    } -> std::convertible_to<typename T::AttributeType>;
    typename T::AttributeType;
};
}  // namespace plato::third_party_integration::boost_log

#endif
