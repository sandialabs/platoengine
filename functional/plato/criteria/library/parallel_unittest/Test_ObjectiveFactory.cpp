#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/criteria/library/ObjectiveFactory.hpp"
#include "plato/services/InternalLoggerConsoleSink.hpp"
#include "plato/test_utilities/Strings.hpp"

namespace plato::criteria::library::parallel_unittest
{
namespace
{
constexpr auto kRootRank = 0;

[[nodiscard]] auto two_group_color() -> int
{
    const auto tWorldCommunicator = boost::mpi::communicator{};
    return tWorldCommunicator.rank() == kRootRank ? 0 : 1;
}
}  // namespace

TEST(ObjectiveFactory, LogAggregateData)
{
    const auto tLogSinkStream = boost::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLoggerSink = services::internal_logger_console_sink(tLogSinkStream);

    const auto tRank = boost::mpi::communicator{}.rank();
    constexpr auto tRootRank = 0;

    const auto tWorldCommunicator = boost::mpi::communicator{};
    ASSERT_GE(tWorldCommunicator.size(), 2U);
    // Split into two groups
    const auto tGroupColor = two_group_color();
    const auto tGroupCommunicator = tWorldCommunicator.split(tGroupColor);

    const auto tNames = std::vector<std::string>{"shoe", "hat", "watch"};
    const auto tAggregateData = detail::AggregationData{
        .mName = tNames.at(tGroupColor),
        .mWeight = 2.125,
        .mNormalization = tRank == tRootRank ? std::optional<double>{} : std::optional<double>{0.25},
        .mGoalScaling = 4.5};

    const auto tAggregateComm = AggregateComm{boost::mpi::communicator{}};
    const auto tObjectiveComm = ObjectiveComm{tGroupCommunicator};
    if (tGroupColor == 0)
    {
        detail::log_aggregate_data(std::vector{tAggregateData}, tAggregateComm, tObjectiveComm);
    }
    else
    {
        auto tAggregateData2 = tAggregateData;
        tAggregateData2.mName = tNames.back();
        detail::log_aggregate_data(std::vector{tAggregateData, tAggregateData2}, tAggregateComm, tObjectiveComm);
    }

    if (tGroupCommunicator.rank() == tRootRank)
    {
        test_utilities::expect_string_contains_substring(tLogSinkStream->str(), tAggregateData.mName,
                                                         TEST_CONTEXT("Objective name"));
        test_utilities::expect_string_contains_substring(
            tLogSinkStream->str(), std::format("{}", tAggregateData.mWeight), TEST_CONTEXT("Weight"));
        test_utilities::expect_string_contains_substring(tLogSinkStream->str(),
                                                         std::format("{}", tAggregateData.mNormalization.value_or(1.0)),
                                                         TEST_CONTEXT("Normalization"));
        test_utilities::expect_string_contains_substring(
            tLogSinkStream->str(), std::format("{}", tAggregateData.mGoalScaling), TEST_CONTEXT("Scaling"));
        test_utilities::expect_string_contains_substring(tLogSinkStream->str(),
                                                         std::format("{: 8.7e}", detail::total_weight(tAggregateData)),
                                                         TEST_CONTEXT("Total"));
    }
    else
    {
        EXPECT_TRUE(tLogSinkStream->str().empty());
    }
    if (tGroupColor == 0)
    {
        test_utilities::expect_string_contains_substring(
            tLogSinkStream->str(), std::format("{}", "objective:aggregator"), TEST_CONTEXT("Equation summary"));
    }
    else if (tGroupColor == 1 && tGroupCommunicator.rank() == tRootRank)
    {
        test_utilities::expect_string_contains_substring(tLogSinkStream->str(), tNames.back(),
                                                         TEST_CONTEXT("Objective name"));
    }
}
}  // namespace plato::criteria::library::parallel_unittest
