#include "plato/third_party_integration/boost_log/Severity.hpp"

#include <boost/log/expressions.hpp>

#include "plato/utilities/EnumTable.hpp"

namespace plato::third_party_integration::boost_log
{
auto operator<<(std::ostream& aStream, const Severity aSeverity) -> std::ostream&
{
    static const auto tSeverityTable = utilities::EnumTable<Severity>({{Severity::kDebug, "debug"},
                                                                       {Severity::kInfo, "info"},
                                                                       {Severity::kWarning, "warning"},
                                                                       {Severity::kError, "error"}});
    const auto tSeverityAsString = tSeverityTable.toString(aSeverity);
    assert(tSeverityAsString);
    aStream << tSeverityAsString.value();
    return aStream;
}

auto SeverityAttribute::formatter() -> boost::log::formatter
{
    namespace ble = boost::log::expressions;
    return boost::log::formatter{
        ble::stream << ble::if_(ble::has_attr(severity_attribute))[ble::stream << "[" << severity_attribute << "] "]};
}

}  // namespace plato::third_party_integration::boost_log
