#ifndef PLATO_INPUTPARSER_UNWRAPPEDBOOSTOPTIONAL
#define PLATO_INPUTPARSER_UNWRAPPEDBOOSTOPTIONAL

#include <boost/fusion/adapted/struct/detail/extension.hpp>
#include <boost/fusion/include/size.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/optional.hpp>
#include <type_traits>

namespace plato::input_parser
{
/// @brief Type trait that removes a boost::optional wrapper from a type and returns type with cvref removed.
template <typename T>
struct UnwrappedBoostOptional
{
    using Type = std::remove_cvref_t<T>;
};

/// @brief Type trait specialization that extracts the value type from boost::optional.
template <typename T>
struct UnwrappedBoostOptional<boost::optional<T>>
{
    using Type = T;
};

/// @brief Alias for the type produced by UnwrappedBoostOptional.
template <typename T>
using UnwrappedBoostOptionalT = typename UnwrappedBoostOptional<std::remove_cvref_t<T>>::Type;

}  // namespace plato::input_parser
#endif
