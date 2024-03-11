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
    namespace pfcd = plato::criteria::library::detail;

    // One objective and three ranks
    const input_parser::objective tObjective;
    EXPECT_TRUE(pfcd::validate_number_of_ranks_vs_objectives({tObjective}).has_value());

    // Three objectives and three ranks
    EXPECT_FALSE(pfcd::validate_number_of_ranks_vs_objectives({tObjective, tObjective, tObjective}).has_value());

    // Four objectives and three ranks
    EXPECT_FALSE(
        pfcd::validate_number_of_ranks_vs_objectives({tObjective, tObjective, tObjective, tObjective}).has_value());
}
}  // namespace plato::criteria::library::unittest
