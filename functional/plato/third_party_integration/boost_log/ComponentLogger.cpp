#include "plato/third_party_integration/boost_log/ComponentLogger.hpp"

#include <boost/log/attributes/constant.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/mpi/communicator.hpp>

#include "plato/third_party_integration/boost_log/ComponentAttributes.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"
#include "plato/third_party_integration/boost_log/TimeStampAttribute.hpp"

namespace plato::third_party_integration::boost_log
{
namespace
{
void log(boost::log::sources::logger& aLogger, const std::string_view aMessage, const Severity aSeverity)
{
    aLogger.add_attribute(kTimeStampAttributeName.data(), boost::log::attributes::local_clock());
    BOOST_LOG_SCOPED_LOGGER_ATTR(aLogger, kSeverityAttributeName.data(),
                                 boost::log::attributes::make_constant(aSeverity))

    BOOST_LOG(aLogger) << aMessage;
}
}  // namespace

ComponentLogger::ComponentLogger(const components::ComponentType aComponentType, const std::string_view aComponentName)
{
    mLogger.add_attribute(kMPIRankAttributeName.data(),
                          boost::log::attributes::make_constant(boost::mpi::communicator{}.rank()));
    mLogger.add_attribute(kLogSourceAttributeName.data(),
                          boost::log::attributes::constant<LogSource>(LogSource::kInternal));
    mLogger.add_attribute(kComponentAttributeName.data(),
                          boost::log::attributes::make_constant(ComponentTypeAndName{
                              .mComponentType = aComponentType, .mComponentName = std::string{aComponentName}}));
}

void ComponentLogger::logDebugMessage(std::string_view aMessage) { log(mLogger, aMessage, Severity::kDebug); }

void ComponentLogger::logInfo(const std::string_view aMessage) { log(mLogger, aMessage, Severity::kInfo); }

void ComponentLogger::logWarning(const std::string_view aMessage) { log(mLogger, aMessage, Severity::kWarning); }

void ComponentLogger::logError(std::string_view aMessage) { log(mLogger, aMessage, Severity::kError); }

}  // namespace plato::third_party_integration::boost_log
