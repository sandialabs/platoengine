#include <gtest/gtest.h>

#include "plato/services/InternalLoggerConsoleSink.hpp"
#include "plato/services/SystemLogger.hpp"
#include "plato/test_utilities/Strings.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::services::parallel_unittest
{
TEST(SystemLogger, SystemLoggerOnDifferentCommunicators)
{
    const auto tStream = boost::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLogSink = internal_logger_console_sink(tStream);

    constexpr auto tObjectiveName = std::string_view{"lens"};
    const auto tCommunicator = boost::mpi::communicator{};
    const auto tGroupColor = tCommunicator.rank() % 2;
    const auto tGroupCommunicator = tCommunicator.split(tGroupColor);

    const auto tTestLogger = [&](auto& aLogger, const test_utilities::TestContext& aTestContext)
    {
        tStream->str(std::string{});
        aLogger.logInfo("Hey there");

        constexpr auto tRootRank = 0;
        if (tGroupCommunicator.rank() == tRootRank)
        {
            test_utilities::expect_string_contains_substring(tStream->str(), tObjectiveName, aTestContext);
        }
        else
        {
            EXPECT_TRUE(tStream->str().empty());
        }
    };

    {
        auto tLogger = SystemLogger{components::ComponentType::kObjective, tObjectiveName, tGroupCommunicator};
        tTestLogger(tLogger, TEST_CONTEXT("Direct construction"));
    }
    {
        auto tLogger = component_logger(components::ComponentType::kObjective, tObjectiveName, tGroupCommunicator);
        tTestLogger(tLogger, TEST_CONTEXT("Component logger function"));
    }
}
}  // namespace plato::services::parallel_unittest
