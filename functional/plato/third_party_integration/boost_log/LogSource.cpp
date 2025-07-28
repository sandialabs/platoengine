#include "plato/third_party_integration/boost_log/LogSource.hpp"

#include <boost/log/expressions.hpp>

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

template <LogSource kIncludedLogSource>
auto LogSourceAttribute<kIncludedLogSource>::filter() -> boost::log::filter
{
    return boost::log::filter{boost::log::expressions::has_attr(log_source_attribute) &&
                              log_source_attribute == kIncludedLogSource};
}

template struct LogSourceAttribute<LogSource::kInternal>;
template struct LogSourceAttribute<LogSource::kExternal>;

}  // namespace plato::third_party_integration::boost_log
