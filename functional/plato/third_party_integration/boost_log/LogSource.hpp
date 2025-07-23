#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_LOGSOURCE
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_LOGSOURCE

#include <boost/log/expressions/keyword.hpp>
#include <ostream>

namespace plato::third_party_integration::boost_log
{
enum struct LogSource
{
    kInternal,
    kExternal
};

auto operator<<(std::ostream& aStream, LogSource aLogSource) -> std::ostream&;

}  // namespace plato::third_party_integration::boost_log

constexpr inline auto kLogSourceAttributeName = std::string_view{"Log source"};

BOOST_LOG_ATTRIBUTE_KEYWORD(log_source,
                            kLogSourceAttributeName.data(),
                            plato::third_party_integration::boost_log::LogSource)

#endif
