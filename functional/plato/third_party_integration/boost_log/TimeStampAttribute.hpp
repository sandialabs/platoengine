#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_TIMESTAMPATTRIBUTE
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_TIMESTAMPATTRIBUTE

#include <boost/log/attributes/clock.hpp>
#include <boost/log/expressions/formatter.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <string_view>

namespace plato::third_party_integration::boost_log
{
/// @brief A formatter for a time stamp in the format: `[y-m-d HH:MM:SS]`
[[nodiscard]] auto time_stamp_formatter() -> boost::log::formatter;

constexpr inline auto kTimeStampAttributeName = std::string_view{"Time stamp"};
}  // namespace plato::third_party_integration::boost_log

BOOST_LOG_ATTRIBUTE_KEYWORD(time_stamp_attribute,
                            plato::third_party_integration::boost_log::kTimeStampAttributeName.data(),
                            boost::log::attributes::local_clock);

#endif
