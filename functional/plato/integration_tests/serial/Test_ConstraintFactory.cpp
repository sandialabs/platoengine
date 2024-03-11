#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/criteria/library/ConstraintValidation.hpp"
#include "plato/main/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::integration_tests::serial
{
TEST(ConstraintFactory, ValidConstraint)
{
    namespace pftu = plato::test_utilities;

    const std::string tConstraintInput = pftu::create_valid_example_constraint_string();
    const std::string tGeometryInput = pftu::create_valid_density_topology_geometry_string();
    const std::string tOptimizerInput = pftu::create_valid_example_optimization_parameters_string();
    const std::string tObjectiveInput = pftu::create_valid_example_objective_string();
    const main::library::ValidatedInput tData =
        main::library::parse_and_validate(tConstraintInput + tGeometryInput + tOptimizerInput + tObjectiveInput);

    ASSERT_EQ(tData.constraints().rawInput().size(), 1);
    const auto tConstraint = criteria::library::detail::make_constraint(tData.constraints().rawInput().front());
    EXPECT_TRUE(tConstraint.mLinear);
    EXPECT_EQ(tConstraint.mConstraintTarget, 13.0);
}
}  // namespace plato::integration_tests::serial
