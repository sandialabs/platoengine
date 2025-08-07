#include <gtest/gtest.h>

#include <boost/shared_ptr.hpp>

#include "plato/services/InternalLoggerConsoleSink.hpp"
#include "plato/services/TaskLogSetupTeardown.hpp"
#include "plato/test_utilities/Strings.hpp"

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
    test_utilities::expect_string_contains_substring(tStream->str(), tSetupMessage, TEST_CONTEXT("Set up message"));
    constexpr auto tExpectedTeardownMessage = std::string_view{"Filtering complete"};
    test_utilities::expect_string_contains_substring(tStream->str(), tExpectedTeardownMessage,
                                                     TEST_CONTEXT("Tear down message"));
}
}  // namespace plato::services::unittest
