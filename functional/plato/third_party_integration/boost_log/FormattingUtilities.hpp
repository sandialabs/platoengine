#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_FORMATTINGUTILITIES
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_FORMATTINGUTILITIES

#include <string>

#include "plato/third_party_integration/boost_log/AttributeTypes.hpp"
#include "plato/utilities/Colorize.hpp"

namespace plato::third_party_integration::boost_log
{
/// @brief Returns the color code escape sequence based on @a aFormattingStyle.
///
/// Mainly, if @a aFormattingStyle is `FormattingStyle::kNone`, the returned string will be empty.
[[nodiscard]] auto color_code(utilities::TextColor aTextColor, FormattingStyle aFormattingStyle) -> std::string;

}  // namespace plato::third_party_integration::boost_log

#endif
