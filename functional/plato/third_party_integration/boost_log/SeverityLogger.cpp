#include "plato/third_party_integration/boost_log/SeverityLogger.hpp"

#include <boost/log/attributes/constant.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/mpi/communicator.hpp>

#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"
#include "plato/third_party_integration/boost_log/TimeStampAttribute.hpp"

namespace plato::third_party_integration::boost_log
{
void SeverityLogger::logMessage(const std::string_view aMessage, const Severity aSeverity)
{
    mLogger.add_attribute(TimeStampAttribute::name().data(), boost::log::attributes::local_clock());
    BOOST_LOG_SCOPED_LOGGER_ATTR(mLogger, SeverityAttribute::name().data(),
                                 boost::log::attributes::make_constant(aSeverity))

    BOOST_LOG(mLogger) << aMessage;
}
}  // namespace plato::third_party_integration::boost_log
