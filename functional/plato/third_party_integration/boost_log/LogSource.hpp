#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_LOGSOURCE
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_LOGSOURCE

#include <boost/log/expressions/filter.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <cstdint>
#include <ostream>

#include "plato/third_party_integration/boost_log/AttributeTypes.hpp"

namespace plato::third_party_integration::boost_log
{
/// @brief Enum tag to distinguish log messages generated internally to platoengine vs. externally in shared libraries.
enum struct LogSource : std::uint8_t
{
    kInternal,
    kExternal
};

/// @brief Stream insertion operator for LogSource.
auto operator<<(std::ostream& aStream, LogSource aLogSource) -> std::ostream&;

/// @brief Attribute for specifying the source of a log message, either internal or external.
/// @tparam kFilteredLogSource Indicates which type of message to pass through the filter.
template <LogSource kIncludedLogSource>
struct LogSourceAttribute
{
    using AttributeType = LogSource;
    AttributeType mValue = kIncludedLogSource;

    [[nodiscard]] constexpr static inline auto name() -> std::string_view;

    /// @brief Returns a filter that filters out all MPI ranks except the root rank on @a aCommunicator.
    [[nodiscard]] static auto filter() -> boost::log::filter;
};

template <LogSource kIncludedLogSource>
constexpr inline auto LogSourceAttribute<kIncludedLogSource>::name() -> std::string_view
{
    return std::string_view{"Log source"};
}

static_assert(AttributeWithFilter<LogSourceAttribute<LogSource::kInternal>>,
              "LogSource must satisfy concept AttributeWithFilter");

}  // namespace plato::third_party_integration::boost_log

BOOST_LOG_ATTRIBUTE_KEYWORD(log_source_attribute,
                            plato::third_party_integration::boost_log::LogSourceAttribute<
                                plato::third_party_integration::boost_log::LogSource::kInternal>::name()
                                .data(),
                            typename plato::third_party_integration::boost_log::LogSource)

#endif
