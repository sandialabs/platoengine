#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_LOGSOURCE
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_LOGSOURCE

#include <boost/log/expressions/filter.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <ostream>

namespace plato::third_party_integration::boost_log
{
/// @brief Enum tag to distinguish log messages generated internally to platoengine vs. externally in shared libraries.
enum struct LogSource
{
    kInternal,
    kExternal
};

/// @brief Returns a filter that filters out messages not matching @a aLogSourceToInclude.
[[nodiscard]] auto log_source_filter(LogSource aLogSourceToInclude) -> boost::log::filter;

auto operator<<(std::ostream& aStream, LogSource aLogSource) -> std::ostream&;

constexpr inline auto kLogSourceAttributeName = std::string_view{"Log source"};

}  // namespace plato::third_party_integration::boost_log

BOOST_LOG_ATTRIBUTE_KEYWORD(log_source_attribute,
                            plato::third_party_integration::boost_log::kLogSourceAttributeName.data(),
                            plato::third_party_integration::boost_log::LogSource)

#endif
