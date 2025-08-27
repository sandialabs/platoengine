#include "plato/third_party_integration/boost_log/Severity.hpp"

#include <boost/log/expressions.hpp>

#include "plato/third_party_integration/boost_log/FormattingUtilities.hpp"
#include "plato/utilities/Colorize.hpp"
#include "plato/utilities/EnumTable.hpp"

namespace plato::third_party_integration::boost_log
{
namespace
{
const auto kSeverityColorMap =
    std::unordered_map<Severity, utilities::TextColor>{{Severity::kDebug, utilities::TextColor::kLightMagenta},
                                                       {Severity::kInfo, utilities::TextColor::kLightGreen},
                                                       {Severity::kWarning, utilities::TextColor::kYellow},
                                                       {Severity::kError, utilities::TextColor::kRed}};

const auto kSeverityTable = utilities::EnumTable<Severity>({{Severity::kDebug, "debug"},
                                                            {Severity::kInfo, "info"},
                                                            {Severity::kWarning, "warning"},
                                                            {Severity::kError, "error"}});

[[nodiscard]] auto severity_color_code(const Severity aSeverityKeyword,
                                       const FormattingStyle aFormattingStyle) -> std::string
{
    return color_code(kSeverityColorMap.at(aSeverityKeyword), aFormattingStyle);
}
}  // namespace

auto operator<<(std::ostream& aStream, const Severity aSeverity) -> std::ostream&
{
    const auto tSeverityAsString = kSeverityTable.toString(aSeverity);
    assert(tSeverityAsString);
    aStream << utilities::colorize(tSeverityAsString.value(), kSeverityColorMap.at(aSeverity));
    return aStream;
}

auto to_string(Severity aSeverity) -> std::string
{
    auto tSeverityAsString = kSeverityTable.toString(aSeverity);
    assert(tSeverityAsString);
    return std::move(tSeverityAsString).value();
}

auto SeverityAttribute::formatter(const FormattingStyle aFormattingStyle) -> boost::log::formatter
{
    namespace ble = boost::log::expressions;

    using StreamType = boost::log::formatter::stream_type;
    const auto tFormatter = [aFormattingStyle](const boost::log::record_view& aRecord, StreamType& aStream)
    {
        if (const auto tSeverity = boost::log::extract<Severity>(SeverityAttribute::name().data(), aRecord))
        {
            const auto tColorCode = severity_color_code(tSeverity.get(), aFormattingStyle);
            aStream << "[" << tColorCode << tSeverity << color_code(utilities::TextColor::kDefault, aFormattingStyle)
                    << "] ";
        }
    };

    return boost::log::formatter{tFormatter};
}

}  // namespace plato::third_party_integration::boost_log
