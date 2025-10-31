#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/test_utilities/TestAppConfigurations.hpp"
#include "plato/services/AppConfigurationUtilities.hpp"

namespace plato::criteria::library::unittest
{
namespace
{
[[nodiscard]] auto test_constraint_inputs() -> std::tuple<input_parser::constraint,
                                                          input_parser::constraint,
                                                          std::vector<services::AppConfigurationWithDirectory>>
{
    const auto [tTestDirectorySetupTeardown, tTestAppName] = test_utilities::test_configurations();
    const auto tAppConfigurations = services::app_configurations({tTestDirectorySetupTeardown.directory()});

    auto tBaseConstraint = input_parser::constraint{};
    tBaseConstraint.app = input_parser::AppName{std::string{tTestAppName}};

    auto tScalarConstraintInput = tBaseConstraint;
    tScalarConstraintInput.criterion = input_parser::CriterionName{"scalar"};
    tScalarConstraintInput.constraint_value = 42.0;

    auto tVectorConstraintInput = tBaseConstraint;
    tVectorConstraintInput.app = input_parser::AppName{std::string{tTestAppName}};
    tVectorConstraintInput.criterion = input_parser::CriterionName{"vector-with-components"};
    tVectorConstraintInput.constraint_value_list = ConstraintValueList{
        {input_parser::ComponentAndTarget{/*.component=*/input_parser::IdentifierString{"asia"}, /*.target=*/43.0},
         input_parser::ComponentAndTarget{/*.component=*/input_parser::IdentifierString{"america"},
                                          /*.target=*/42.0}}};
    return std::make_tuple(std::move(tScalarConstraintInput), std::move(tVectorConstraintInput), tAppConfigurations);
}
}  // namespace

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
    const auto [tScalarConstraintInput, tVectorConstraintInput, tAppConfigurations] = test_constraint_inputs();

    {
        const auto& tScalarConfiguration = tAppConfigurations.front().mConfiguration.mCriteria.front();
        const auto tConstraintTargets = detail::make_constraint_target(tScalarConstraintInput, tScalarConfiguration);
        ASSERT_EQ(tConstraintTargets.size(), 1U);
        EXPECT_EQ(tConstraintTargets.value(tConstraintTargets.size()).front(), 42.0);
    }
    {
        const auto& tVectorConfiguration = tAppConfigurations.front().mConfiguration.mCriteria.back();
        const auto tConstraintTargets = detail::make_constraint_target(tVectorConstraintInput, tVectorConfiguration);
        ASSERT_EQ(tConstraintTargets.size(), 2U);
        EXPECT_EQ(tConstraintTargets.value(tConstraintTargets.size()).front(), 42.0);
        EXPECT_EQ(tConstraintTargets.value(tConstraintTargets.size()).back(), 43.0);
    }
}

TEST(ConstraintFactory, ConstraintComponentIndices)
{
    const auto [tScalarConstraintInput, tVectorConstraintInput, tAppConfigurations] = test_constraint_inputs();
    {
        const auto& tScalarConfiguration = tAppConfigurations.front().mConfiguration.mCriteria.front();
        const auto tConstraintComponentIndices =
            detail::constraint_component_indices(tScalarConstraintInput, tScalarConfiguration);
        EXPECT_FALSE(tConstraintComponentIndices);
    }
    {
        const auto& tVectorConfiguration = tAppConfigurations.front().mConfiguration.mCriteria.back();
        const auto tConstraintComponentIndices =
            detail::constraint_component_indices(tVectorConstraintInput, tVectorConfiguration);
        ASSERT_TRUE(tConstraintComponentIndices);
        const auto tExpected = std::set<std::size_t>{0U, 2U};
        EXPECT_EQ(tExpected, tConstraintComponentIndices.value());
    }
}

}  // namespace plato::criteria::library::unittest
