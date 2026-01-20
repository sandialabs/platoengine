#include <gtest/gtest.h>

#include <boost/smart_ptr/shared_ptr.hpp>

#include "plato/components/ComponentTypeStream.hpp"
#include "plato/core/Function.hpp"
#include "plato/criteria/library/LoggingFunction.hpp"
#include "plato/linear_algebra/DynamicVectorFormatter.hpp"
#include "plato/services/InternalLoggerConsoleSink.hpp"
#include "plato/test_utilities/Strings.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::criteria::library::unittest
{
namespace
{
using FunctionInfo = core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFunction>;
using GradientInfo = core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFirstDerivative>;
using TestFunction = core::Function<double, FunctionInfo, GradientInfo>;
}  // namespace

TEST(LoggingFunction, MakeLoggingFunctionWithCommunicator)
{
    constexpr auto tOutputDimension = 3U;
    const auto tTestFunction = [](const double aArgument)
    { return linear_algebra::DynamicVector(tOutputDimension, aArgument); };
    const auto tFunction = TestFunction{tTestFunction, tTestFunction};

    const auto tFunctionName = std::string{"vector-fun"};
    constexpr auto tComponentType = components::ComponentType::kConstraint;
    const auto tCommunicator = boost::mpi::communicator{};
    const auto tGroupColor = tCommunicator.rank() % 2;
    const auto tGroupCommunicator = tCommunicator.split(tGroupColor);
    const auto tLoggingFunction = make_logging_function(tFunction, tComponentType, tFunctionName, tGroupCommunicator);

    const auto tLogSinkStream = boost::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLoggerSink = services::internal_logger_console_sink(tLogSinkStream);

    constexpr auto tArgument = 42.0;
    EXPECT_EQ(tFunction.evaluate<core::evaluation::kFunction>(tArgument),
              tLoggingFunction.evaluate<core::evaluation::kFunction>(tArgument));
    EXPECT_EQ(tFunction.evaluate<core::evaluation::kFirstDerivative>(tArgument),
              tLoggingFunction.evaluate<core::evaluation::kFirstDerivative>(tArgument));

    constexpr auto tRootRank = 0;
    if (tGroupCommunicator.rank() == tRootRank)
    {
        test_utilities::expect_string_contains_substring(tLogSinkStream->str(), tFunctionName,
                                                         TEST_CONTEXT("Checking function name"));
        test_utilities::expect_string_contains_substring(tLogSinkStream->str(), components::to_string(tComponentType),
                                                         TEST_CONTEXT("Checking component type"));
        test_utilities::expect_string_contains_substring(tLogSinkStream->str(), "Gradient",
                                                         TEST_CONTEXT("Checking for Gradient"));
    }
    else
    {
        EXPECT_TRUE(tLogSinkStream->str().empty());
    }
}
}  // namespace plato::criteria::library::unittest
