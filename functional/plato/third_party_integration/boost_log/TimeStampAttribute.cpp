#include "plato/third_party_integration/boost_log/TimeStampAttribute.hpp"

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/log/expressions.hpp>  // NOLINT(misc-include-cleaner)
#include <boost/log/expressions/formatter.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/expressions/formatters/stream.hpp>
#include <boost/log/support/date_time.hpp>  // NOLINT(misc-include-cleaner)
#include <string>

#include "plato/third_party_integration/boost_log/AttributeTypes.hpp"
#include "plato/third_party_integration/boost_log/FormattingUtilities.hpp"
#include "plato/utilities/Colorize.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::third_party_integration::boost_log
{
[[nodiscard]] auto time_format(const FormattingStyle aFormattingStyle) -> std::string
{
    constexpr auto tTimeFormat = std::string_view{"%Y-%m-%d %H:%M:%S"};
    return utilities::concatenate('[', color_code(utilities::TextColor::kDarkGray, aFormattingStyle), tTimeFormat,
                                  color_code(utilities::TextColor::kDefault, aFormattingStyle), "] ");
}

auto TimeStampAttribute::formatter(const FormattingStyle aFormattingStyle) -> boost::log::formatter
{
    namespace ble = boost::log::expressions;
    return boost::log::formatter{
        ble::stream << ble::format_date_time<boost::posix_time::ptime>(name().data(), time_format(aFormattingStyle))};
}
}  // namespace plato::third_party_integration::boost_log
