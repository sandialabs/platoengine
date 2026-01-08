#include <gtest/gtest.h>

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/output/OutputManager.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/IndexRange.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::output
{
struct OutputInfo;
}

namespace plato::output::unittest
{
namespace
{
constexpr std::size_t kNumberOfOutputCalls = 5U;

using ExpectedNumberOfAppends = utilities::NamedType<std::size_t, struct ExpectedNumberOfAppendsTag>;
using ExpectedNumberOfOverwrites = utilities::NamedType<std::size_t, struct ExpectedNumberOfOverwritesTag>;

void test_number_of_output_calls_matches_expected(const OutputMode aOutputMode,
                                                  const ExpectedNumberOfOverwrites aExpectedNumberOfOverwrites,
                                                  const ExpectedNumberOfAppends aExpectedNumberOfAppends,
                                                  const test_utilities::TestContext& aTestContext)
{
    auto tNumberOfOverwrites = std::size_t{0U};
    auto tNumberOfAppends = std::size_t{0U};
    auto tLoggingOutputFunction =
        [&tNumberOfAppends, &tNumberOfOverwrites](const linear_algebra::DynamicVector<double>&,
                                                  const output::OutputInfo& aOutputInfo)
    {
        if (aOutputInfo.mOverwrite)
        {
            ++tNumberOfOverwrites;
        }
        else
        {
            ++tNumberOfAppends;
        }
    };
    auto tOutputManager = OutputManager{tLoggingOutputFunction, aOutputMode};

    const linear_algebra::DynamicVector<double> tDummyDesignVariables{};
    for ([[maybe_unused]] const auto tIteration : utilities::IndexRange{kNumberOfOutputCalls})
    {
        tOutputManager.output(tDummyDesignVariables);
    }

    EXPECT_EQ(tNumberOfOverwrites, aExpectedNumberOfOverwrites.mValue) << aTestContext;
    EXPECT_EQ(tNumberOfAppends, aExpectedNumberOfAppends.mValue) << aTestContext;
}
}  // namespace

TEST(OutputManager, WriteNever)
{
    constexpr auto tExpectedNumberOfOverwriteCalls = ExpectedNumberOfOverwrites{0U};
    constexpr auto tExpectedNumberOfAppendCalls = ExpectedNumberOfAppends{0U};
    test_number_of_output_calls_matches_expected(OutputMode::kNever, tExpectedNumberOfOverwriteCalls,
                                                 tExpectedNumberOfAppendCalls, TEST_CONTEXT("Never write"));
}

TEST(OutputManager, WriteOverwriteAllSteps)
{
    constexpr auto tExpectedNumberOfOverwriteCalls = ExpectedNumberOfOverwrites{kNumberOfOutputCalls};
    constexpr auto tExpectedNumberOfAppendCalls = ExpectedNumberOfAppends{0U};
    test_number_of_output_calls_matches_expected(OutputMode::kEveryIterationOverwrite, tExpectedNumberOfOverwriteCalls,
                                                 tExpectedNumberOfAppendCalls, TEST_CONTEXT("Overwrite every"));
}

TEST(OutputManager, WriteOnEveryIterationAndFinalOutput)
{
    constexpr auto tExpectedNumberOfOverwriteCalls = ExpectedNumberOfOverwrites{1U};
    constexpr auto tExpectedNumberOfAppendCalls = ExpectedNumberOfAppends{kNumberOfOutputCalls - 1};
    test_number_of_output_calls_matches_expected(OutputMode::kEveryIterationAppend, tExpectedNumberOfOverwriteCalls,
                                                 tExpectedNumberOfAppendCalls, TEST_CONTEXT("Append every"));
}
}  // namespace plato::output::unittest
