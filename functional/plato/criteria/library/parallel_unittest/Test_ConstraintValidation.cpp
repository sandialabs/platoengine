#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/criteria/library/ConstraintValidation.hpp"

namespace plato::criteria::library::parallel_unittest
{
TEST(ConstraintValidation, NumberOfProcessors)
{
    const auto tNumberOfWorldRanks = boost::mpi::communicator{}.size();
    auto tConstraint = input_parser::constraint{};

    EXPECT_FALSE(detail::validate_constraint_number_of_processors(tConstraint).has_value())
        << "Default, assumes one rank";

    tConstraint.number_of_processors = tNumberOfWorldRanks + 10U;
    EXPECT_TRUE(detail::validate_constraint_number_of_processors(tConstraint).has_value())
        << "Exceeds number of ranks on comm world";

    std::cout << "Message: " << detail::validate_constraint_number_of_processors(tConstraint).value() << std::endl;

    tConstraint.number_of_processors = tNumberOfWorldRanks;
    EXPECT_FALSE(detail::validate_constraint_number_of_processors(tConstraint).has_value()) << "Uses all ranks";

    tConstraint.number_of_processors = tNumberOfWorldRanks - 1U;
    EXPECT_FALSE(detail::validate_constraint_number_of_processors(tConstraint).has_value())
        << "Uses one fewer rank than all";
}
}  // namespace plato::criteria::library::parallel_unittest
