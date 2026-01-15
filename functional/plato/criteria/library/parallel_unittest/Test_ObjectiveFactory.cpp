#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/criteria/library/ObjectiveFactory.hpp"
#include "plato/services/InternalLoggerConsoleSink.hpp"
#include "plato/test_utilities/Strings.hpp"

namespace plato::criteria::library::parallel_unittest
{
TEST(ObjectiveFactory, LogAggregateData)
{
    const auto tLogSinkStream = boost::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLoggerSink = services::internal_logger_console_sink(tLogSinkStream);

    const auto tRank = boost::mpi::communicator{}.rank();
    constexpr auto tRootRank = 0;

    ASSERT_LE(boost::mpi::communicator{}.size(), 3U);
    const auto tNames = std::map<int, std::string>{{0, "shoe"}, {1, "hat"}, {2, "watch"}};
    const auto tAggregateData = detail::AggregationData{
        .mName = std::format("baloney-{}", tNames.at(tRank)),
        .mWeight = 2.125,
        .mNormalization = tRank == tRootRank ? std::optional<double>{} : std::optional<double>{0.25},
        .mGoalScaling = 4.5};

    detail::log_aggregate_data(std::vector{tAggregateData}, boost::mpi::communicator{});

    test_utilities::expect_string_contains_substring(tLogSinkStream->str(), tAggregateData.mName,
                                                     TEST_CONTEXT("Objective name"));
    test_utilities::expect_string_contains_substring(tLogSinkStream->str(), std::format("{}", tAggregateData.mWeight),
                                                     TEST_CONTEXT("Weight"));
    test_utilities::expect_string_contains_substring(tLogSinkStream->str(),
                                                     std::format("{}", tAggregateData.mNormalization.value_or(1.0)),
                                                     TEST_CONTEXT("Normalization"));
    test_utilities::expect_string_contains_substring(
        tLogSinkStream->str(), std::format("{}", tAggregateData.mGoalScaling), TEST_CONTEXT("Scaling"));

    test_utilities::expect_string_contains_substring(
        tLogSinkStream->str(), std::format("{: 8.7e}", detail::total_weight(tAggregateData)), TEST_CONTEXT("Total"));

    if (tRank == tRootRank)
    {
        test_utilities::expect_string_contains_substring(
            tLogSinkStream->str(), std::format("{}", "objective:aggregator"), TEST_CONTEXT("Equation summary"));
    }
}
}  // namespace plato::criteria::library::parallel_unittest
