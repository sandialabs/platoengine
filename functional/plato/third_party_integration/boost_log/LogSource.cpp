#include "plato/third_party_integration/boost_log/LogSource.hpp"

#include "plato/utilities/EnumTable.hpp"

namespace plato::third_party_integration::boost_log
{
auto operator<<(std::ostream& aStream, const LogSource aLogSource) -> std::ostream&
{
    static const auto tLogSourceTable =
        utilities::EnumTable<LogSource>({{LogSource::kInternal, "internal"}, {LogSource::kExternal, "external"}});
    const auto tLogSourceAsString = tLogSourceTable.toString(aLogSource);
    assert(tLogSourceAsString);
    aStream << tLogSourceAsString.value();
    return aStream;
}
}  // namespace plato::third_party_integration::boost_log
