#include "plato/services/TaskLogSetupTeardown.hpp"

namespace plato::services
{

TaskLogSetupTeardown::TaskLogSetupTeardown(std::string aTaskMessage, SystemLogger&& aLogger)
    : mTaskMessage{std::move(aTaskMessage)}, mLogger{std::move(aLogger)}
{
    mLogger.logInfo(mTaskMessage);
}

TaskLogSetupTeardown::~TaskLogSetupTeardown() { mLogger.logInfo(mTaskMessage + " complete"); }

auto jacobian_task_message() -> std::string { return std::string{"Computing vector-Jacobian product"}; }

auto adjoint_jacobian_task_message() -> std::string { return "Computing vector-adjoint-Jacobian product"; }

}  // namespace plato::services
