#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::integration_tests::serial
{
TEST(ConstraintFactory, ValidConstraint)
{
    namespace pftu = plato::test_utilities;

    const auto tData = process_manager::library::make_validated_input(
        test_utilities::create_valid_density_topology_geometry() | test_utilities::create_valid_example_objective() |
        test_utilities::create_valid_example_constraint() | test_utilities::create_valid_example_rol_optimization() |
        test_utilities::create_valid_identity_filter());

    ASSERT_EQ(tData.constraints().rawInput().size(), 1);
    const auto tConstraint = criteria::library::detail::make_constraint(tData.constraints().rawInput().front());
    EXPECT_TRUE(tConstraint.mLinear);
    EXPECT_EQ(tConstraint.mConstraintTarget, 0.0);
}
}  // namespace plato::integration_tests::serial
