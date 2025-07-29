#include "plato/third_party_integration/boost_log/TimeStampAttribute.hpp"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>

#include "plato/utilities/Colorize.hpp"

namespace plato::third_party_integration::boost_log
{
auto TimeStampAttribute::formatter() -> boost::log::formatter
{
    namespace ble = boost::log::expressions;
    return boost::log::formatter{
        ble::stream << ble::format_date_time<boost::posix_time::ptime>(name().data(), "[%Y-%m-%d %H:%M:%S] ")};
}
}  // namespace plato::third_party_integration::boost_log
