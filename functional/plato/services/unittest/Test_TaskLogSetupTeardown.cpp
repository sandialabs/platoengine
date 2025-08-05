#include <gtest/gtest.h>

#include <boost/shared_ptr.hpp>

#include "plato/services/InternalLoggerConsoleSink.hpp"
#include "plato/services/TaskLogSetupTeardown.hpp"

namespace plato::services::unittest
{
TEST(TaskLogSetupTeardown, SetupTeardown)
{
    const auto tStream = boost::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLogSink = internal_logger_console_sink(tStream);

    constexpr auto tSetupMessage = std::string_view{"Filtering"};
    {
        [[maybe_unused]] const auto tLoggerMessageSetupTeardown = TaskLogSetupTeardown{
            std::string{tSetupMessage}, component_logger(components::ComponentType::kFilter, "some-component")};
    }
    EXPECT_NE(tStream->str().find(tSetupMessage), std::string::npos) << "Result: " << tStream->str();
    constexpr auto tExpectedTeardownMessage = std::string_view{"Filtering complete"};
    EXPECT_NE(tStream->str().find(tExpectedTeardownMessage), std::string::npos) << "Result: " << tStream->str();
}
}  // namespace plato::services::unittest
