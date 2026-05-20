#ifndef PLATO_SERVICES_TASK_LOG_SETUP_TEARDOWN
#define PLATO_SERVICES_TASK_LOG_SETUP_TEARDOWN

#include <string>

#include "plato/services/SystemLogger.hpp"

namespace plato::services
{
/// @brief RAII-style object used to log a message on construction and destruction to log scope bounds.
///
/// Facilitates a typical use-case of logging a message on entrance and exit from a function.
class [[nodiscard]] TaskLogSetupTeardown
{
   public:
    /// @param aTaskMessage The message to log on construction, immediately after this function is called. The message
    /// logged on destruction will be @a aTaskMessage appended with "complete".
    TaskLogSetupTeardown(std::string aTaskMessage, SystemLogger&& aLogger);

    ~TaskLogSetupTeardown();

    TaskLogSetupTeardown(const TaskLogSetupTeardown&) = delete;
    TaskLogSetupTeardown(TaskLogSetupTeardown&&) = delete;
    auto operator=(const TaskLogSetupTeardown&) -> TaskLogSetupTeardown& = delete;
    auto operator=(TaskLogSetupTeardown&&) -> TaskLogSetupTeardown& = delete;

   private:
    std::string mTaskMessage;
    SystemLogger mLogger;
};

/// @brief Provides a common message for Jacobian operations
[[nodiscard]] auto jacobian_task_message() -> std::string;

/// @brief Provides a common message for adjoint Jacobian operations
[[nodiscard]] auto adjoint_jacobian_task_message() -> std::string;

}  // namespace plato::services

#endif
