#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_SEVERITY
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_SEVERITY

#include <boost/log/expressions/formatter.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <ostream>

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

constexpr inline auto kSeverityAttributeName = std::string_view{"Severity"};

/// @brief Returns a boost formatter that formats the Severity enum.
[[nodiscard]] auto severity_attribute_formatter() -> boost::log::formatter;

}  // namespace plato::third_party_integration::boost_log

BOOST_LOG_ATTRIBUTE_KEYWORD(severity_attribute,
                            plato::third_party_integration::boost_log::kSeverityAttributeName.data(),
                            plato::third_party_integration::boost_log::Severity)

#endif
