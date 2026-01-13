#include <gtest/gtest.h>

#include <boost/smart_ptr/shared_ptr.hpp>

#include "plato/components/ComponentTypeStream.hpp"
#include "plato/core/Function.hpp"
#include "plato/criteria/library/LoggingFunction.hpp"
#include "plato/services/InternalLoggerConsoleSink.hpp"
#include "plato/test_utilities/Strings.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::criteria::library::unittest
{
namespace
{
using FunctionInfo = core::FunctionInfo<int, core::evaluation::kFunction>;
using GradientInfo = core::FunctionInfo<int, core::evaluation::kFirstDerivative>;
using TestFunction = core::Function<int, FunctionInfo, GradientInfo>;
}  // namespace

TEST(LoggingFunction, MakeLoggingFunction)
{
    const auto tFunction =
        TestFunction{[](const int aArgument) { return aArgument; }, [](const int aArgument) { return 2 * aArgument; }};
    const auto tFunctionName = std::string{"test-func"};
    constexpr auto tComponentType = components::ComponentType::kObjective;
    const auto tLoggingFunction = make_logging_function(tFunction, tComponentType, tFunctionName);

    const auto tLogSinkStream = boost::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLoggerSink = services::internal_logger_console_sink(tLogSinkStream);

    constexpr auto tArgument = 67;
    EXPECT_EQ(tFunction.evaluate<core::evaluation::kFunction>(tArgument),
              tLoggingFunction.evaluate<core::evaluation::kFunction>(tArgument));
    EXPECT_EQ(tFunction.evaluate<core::evaluation::kFirstDerivative>(tArgument),
              tLoggingFunction.evaluate<core::evaluation::kFirstDerivative>(tArgument));

    test_utilities::expect_string_contains_substring(tLogSinkStream->str(), tFunctionName,
                                                     TEST_CONTEXT("Checking function name"));
    test_utilities::expect_string_contains_substring(tLogSinkStream->str(), components::to_string(tComponentType),
                                                     TEST_CONTEXT("Checking component type"));
}
}  // namespace plato::criteria::library::unittest
