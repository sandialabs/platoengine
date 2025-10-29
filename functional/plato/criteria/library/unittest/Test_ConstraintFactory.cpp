#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/test_utilities/TestAppConfigurations.hpp"
#include "plato/services/AppConfigurationUtilities.hpp"

namespace plato::criteria::library::unittest
{

TEST(ConstraintFactory, SizedDualVector)
{
    constexpr unsigned int tSize = 10;
    const linear_algebra::DynamicVector<double> tDualVector = criteria::library::make_dual_vector(tSize);
    ASSERT_EQ(tDualVector.size(), tSize);
    EXPECT_EQ(tDualVector.stdVector().front(), 1.0);
    EXPECT_EQ(tDualVector.stdVector().back(), 1.0);
}

TEST(ConstraintFactory, MakeConstraintTargets)
{
    const auto [tTestDirectorySetupTeardown, tTestAppName] = test_utilities::test_configurations();
    const auto tAppConfigurations = services::app_configurations({tTestDirectorySetupTeardown.directory()});

    auto tBaseConstraint = input_parser::constraint{};
    tBaseConstraint.app = input_parser::AppName{std::string{tTestAppName}};

    {
        auto tConstraintInput = tBaseConstraint;
        tConstraintInput.criterion = input_parser::CriterionName{"scalar"};
        tConstraintInput.constraint_value = 42.0;

        const auto tConstraintTargets = detail::make_constraint_target(tConstraintInput, tAppConfigurations);

        ASSERT_EQ(tConstraintTargets.size(), 1U);
        EXPECT_EQ(tConstraintTargets.value(tConstraintTargets.size()).front(), 42.0);
    }
    {
        auto tConstraintInput = tBaseConstraint;
        tConstraintInput.app = input_parser::AppName{std::string{tTestAppName}};
        tConstraintInput.criterion = input_parser::CriterionName{"vector-with-components"};
        tConstraintInput.constraint_value_list = ConstraintValueList{
            {input_parser::ComponentAndTarget{/*.component=*/input_parser::IdentifierString{"asia"}, /*.target=*/43.0},
             input_parser::ComponentAndTarget{/*.component=*/input_parser::IdentifierString{"america"},
                                              /*.target=*/42.0}}};

        const auto tConstraintTargets = detail::make_constraint_target(tConstraintInput, tAppConfigurations);

        ASSERT_EQ(tConstraintTargets.size(), 2U);
        EXPECT_EQ(tConstraintTargets.value(tConstraintTargets.size()).front(), 42.0);
        EXPECT_EQ(tConstraintTargets.value(tConstraintTargets.size()).back(), 43.0);
    }
}

}  // namespace plato::criteria::library::unittest
