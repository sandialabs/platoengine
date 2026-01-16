#include <gtest/gtest.h>

#include <boost/smart_ptr/shared_ptr.hpp>

#include "plato/components/ComponentTypeStream.hpp"
#include "plato/core/Function.hpp"
#include "plato/criteria/library/LoggingFunction.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/services/InternalLoggerConsoleSink.hpp"
#include "plato/test_utilities/Strings.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::criteria::library::unittest
{
namespace
{
[[nodiscard]] auto log_output(const auto& aTestFunction,
                              const auto& aArgument,
                              const test_utilities::TestContext& aTestContext) -> std::string
{
    const auto tFunctionName = std::string{"test-func"};
    constexpr auto tComponentType = components::ComponentType::kObjective;
    const auto tLoggingFunction = make_logging_function(aTestFunction, tComponentType, tFunctionName);

    const auto tLogSinkStream = boost::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLoggerSink = services::internal_logger_console_sink(tLogSinkStream);

    EXPECT_EQ(aTestFunction.template evaluate<core::evaluation::kFunction>(aArgument),
              tLoggingFunction.template evaluate<core::evaluation::kFunction>(aArgument))
        << aTestContext;
    EXPECT_EQ(aTestFunction.template evaluate<core::evaluation::kFirstDerivative>(aArgument),
              tLoggingFunction.template evaluate<core::evaluation::kFirstDerivative>(aArgument))
        << aTestContext;

    test_utilities::expect_string_contains_substring(tLogSinkStream->str(), tFunctionName,
                                                     EXTEND_CONTEXT("Checking function name", aTestContext));
    test_utilities::expect_string_contains_substring(tLogSinkStream->str(), components::to_string(tComponentType),
                                                     EXTEND_CONTEXT("Checking component type", aTestContext));

    return tLogSinkStream->str();
}
}  // namespace

TEST(LoggingFunction, MakeLoggingFunction)
{
    using FunctionInfo = core::FunctionInfo<int, core::evaluation::kFunction>;
    using GradientInfo = core::FunctionInfo<int, core::evaluation::kFirstDerivative>;
    using TestFunction = core::Function<int, FunctionInfo, GradientInfo>;

    const auto tFunction =
        TestFunction{[](const int aArgument) { return aArgument; }, [](const int aArgument) { return 2 * aArgument; }};
    const auto tArgument = 67;
    const auto tLogText = log_output(tFunction, tArgument, TEST_CONTEXT("Int function"));

    test_utilities::expect_string_contains_substring(tLogText, std::to_string(tArgument),
                                                     TEST_CONTEXT("Checking for result"));
}

TEST(LoggingFunction, GradientNorm)
{
    using Domain = const linear_algebra::DynamicVector<double>&;
    using FunctionInfo = core::FunctionInfo<double, core::evaluation::kFunction>;
    using GradientInfo = core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFirstDerivative>;
    using TestFunction = core::Function<Domain, FunctionInfo, GradientInfo>;

    const auto tFunction = TestFunction{[](Domain aArgument) { return aArgument.stdVector()[0]; },
                                        [](Domain aArgument) { return aArgument; }};

    const auto tArgument = linear_algebra::DynamicVector<double>{1.0, 2.0, 3.0};
    const auto tLogText = log_output(tFunction, tArgument, TEST_CONTEXT("Int function"));

    const auto tNorm = std::sqrt(1.0 + 4.0 + 9.0);
    test_utilities::expect_string_contains_substring(
        tLogText, std::vformat(detail::kFormatSpecification<double>, std::make_format_args(tNorm)),
        TEST_CONTEXT("Checking for norm"));
}
}  // namespace plato::criteria::library::unittest
