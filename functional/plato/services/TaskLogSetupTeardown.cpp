#include "plato/services/TaskLogSetupTeardown.hpp"

namespace plato::services
{

TaskLogSetupTeardown::TaskLogSetupTeardown(std::string aTaskMessage, ComponentLogger&& aLogger)
    : mTaskMessage{std::move(aTaskMessage)}, mLogger{std::move(aLogger)}
{
    mLogger.logInfo(mTaskMessage);
}

TaskLogSetupTeardown::~TaskLogSetupTeardown() { mLogger.logInfo(mTaskMessage + " complete"); }

}  // namespace plato::services
