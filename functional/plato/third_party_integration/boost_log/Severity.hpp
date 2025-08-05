#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_SEVERITY
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_SEVERITY

#include <boost/log/expressions/formatter.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <ostream>

#include "plato/third_party_integration/boost_log/AttributeTypes.hpp"
#include "plato/utilities/EnumTable.hpp"

namespace plato::third_party_integration::boost_log
{
/// @brief Log severity level, used for filtering log messages.
enum struct Severity
{
    kDebug,
    kInfo,
    kWarning,
    kError
};

/// @brief Stream insertion operator for Severity.
auto operator<<(std::ostream& aStream, Severity aSeverity) -> std::ostream&;

/// @brief An attribute for a log message's severity.
struct SeverityAttribute
{
    using AttributeType = Severity;
    AttributeType mValue;

    [[nodiscard]] constexpr static auto name() -> std::string_view;
    [[nodiscard]] static auto formatter(FormattingStyle aFormattingStyle) -> boost::log::formatter;
};

constexpr auto SeverityAttribute::name() -> std::string_view { return std::string_view{"Severity"}; }

static_assert(AttributeWithFormatter<SeverityAttribute>,
              "SeverityAttribute must satisfy concept AttributeWithFormatter");

}  // namespace plato::third_party_integration::boost_log

BOOST_LOG_ATTRIBUTE_KEYWORD(severity_attribute,
                            plato::third_party_integration::boost_log::SeverityAttribute::name().data(),
                            plato::third_party_integration::boost_log::Severity)

#endif
