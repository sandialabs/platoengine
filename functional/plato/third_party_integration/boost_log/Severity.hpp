#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_SEVERITY
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_SEVERITY

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

}  // namespace plato::third_party_integration::boost_log

#endif
