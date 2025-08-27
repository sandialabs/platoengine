#include "plato/third_party_integration/boost_log/FormattingUtilities.hpp"

namespace plato::third_party_integration::boost_log
{
auto color_code(const utilities::TextColor aTextColor, const FormattingStyle aFormattingStyle) -> std::string
{
    if (aFormattingStyle == FormattingStyle::kColor)
    {
        return utilities::color_code(aTextColor);
    }
    return std::string{};
}
}  // namespace plato::third_party_integration::boost_log
