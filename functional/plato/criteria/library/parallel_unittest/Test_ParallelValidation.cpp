#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/criteria/library/ObjectiveValidation.hpp"
#include "plato/input_parser/InputBlocks.hpp"

namespace plato::criteria::library::unittest
{
namespace
{
constexpr auto kNumRanks = int{3};
}

TEST(ParallelObjectiveValidation, MPISize)
{
    auto tComm = boost::mpi::communicator{};
    EXPECT_EQ(tComm.size(), kNumRanks);
}

TEST(ParallelObjectiveValidation, ValidateMPIRanksVsNumberOfObjectives)
{
    // One objective and three ranks
    const input_parser::objective tObjective;
    EXPECT_TRUE(detail::validate_number_of_ranks_vs_serial_objectives({tObjective}).has_value());

    // Three objectives and three ranks
    EXPECT_FALSE(detail::validate_number_of_ranks_vs_serial_objectives({tObjective, tObjective, tObjective}).has_value());

    // Four objectives and three ranks
    EXPECT_FALSE(
        detail::validate_number_of_ranks_vs_serial_objectives({tObjective, tObjective, tObjective, tObjective}).has_value());
}

TEST(ParallelObjectiveValidation, ValidateMPIRanksVsNumberOfObjectivesParallelObjectives)
{
    input_parser::objective tObjective;

    // Number of processors matches the number of ranks for one objective
    tObjective.number_of_processors = static_cast<unsigned int>(kNumRanks);
    EXPECT_FALSE(detail::validate_number_of_ranks_vs_parallel_objectives({tObjective}).has_value());

    // Mismatch with one objective
    tObjective.number_of_processors = static_cast<unsigned int>(kNumRanks + 1);
    EXPECT_TRUE(detail::validate_number_of_ranks_vs_parallel_objectives({tObjective}).has_value());
    tObjective.number_of_processors = static_cast<unsigned int>(kNumRanks - 1);
    EXPECT_TRUE(detail::validate_number_of_ranks_vs_parallel_objectives({tObjective}).has_value());

    // Number of processors matches the number of ranks for two objectives
    auto tObjectiveOneProcessor = tObjective;
    tObjectiveOneProcessor.number_of_processors = 1u;
    EXPECT_FALSE(detail::validate_number_of_ranks_vs_parallel_objectives({tObjective, tObjectiveOneProcessor}).has_value());

    // Mismatch with two objectives
    tObjectiveOneProcessor.number_of_processors = 2u;
    EXPECT_TRUE(detail::validate_number_of_ranks_vs_parallel_objectives({tObjective, tObjectiveOneProcessor}).has_value());
}
}  // namespace plato::criteria::library::unittest
