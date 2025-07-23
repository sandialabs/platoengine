#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_LOGSOURCE
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_LOGSOURCE

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

#endif
