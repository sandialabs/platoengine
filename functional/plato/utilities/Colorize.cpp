#include "plato/utilities/Colorize.hpp"

#include <unistd.h>

#include <iostream>

#include "plato/utilities/EnumTable.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::utilities::detail
{
namespace
{
[[nodiscard]] auto stream_buffer_as_file(const std::ostream& aStream) -> FILE*
{
    const auto* const tStreamBuffer = aStream.rdbuf();
    if (tStreamBuffer == std::cout.rdbuf())
    {
        return stdout;
    }
    else if (tStreamBuffer == std::cerr.rdbuf())
    {
        return stderr;
    }
    return nullptr;
}
}  // namespace

[[nodiscard]] auto stream_supports_color(const std::ostream& aStream) -> bool
{
    const auto tFile = stream_buffer_as_file(aStream);
    return tFile == nullptr ? false : isatty(fileno(tFile));
}

[[nodiscard]] auto color_code(const TextColor aTextColor) -> std::string
{
    const static auto tColorEscapeValueTable = utilities::EnumTable<TextColor>{
        {TextColor::kDefault, "39"},      {TextColor::kRed, "31"},         {TextColor::kGreen, "32"},
        {TextColor::kBlue, "34"},         {TextColor::kCyan, "36"},        {TextColor::kMagenta, "35"},
        {TextColor::kYellow, "33"},       {TextColor::kBlack, "30"},       {TextColor::kLightRed, "91"},
        {TextColor::kLightGreen, "92"},   {TextColor::kLightBlue, "94"},   {TextColor::kLightCyan, "96"},
        {TextColor::kLightMagenta, "95"}, {TextColor::kLightYellow, "93"}, {TextColor::kLightGray, "37"},
        {TextColor::kDarkGray, "90"}};

    constexpr auto tEscapeSequence = std::string_view{"\033["};
    const auto tColorCode = tColorEscapeValueTable.toString(aTextColor);
    assert(tColorCode.has_value());
    return concatenate(tEscapeSequence, tColorCode.value(), "m");
}
}  // namespace plato::utilities::detail
