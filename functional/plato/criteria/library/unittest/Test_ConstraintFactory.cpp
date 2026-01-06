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

TEST(ConstraintTarget, MakeConstraintTargetValue)
{
    const auto [tScalarConstraintInput, tVectorConstraintInput, tAppConfigurations] = test_constraint_inputs();

    // Scalar
    {
        const auto& tScalarConfiguration = tAppConfigurations.front().mConfiguration.mCriteria.front();
        const auto tConstraintTargets =
            detail::make_constraint_target_value(tScalarConstraintInput, tScalarConfiguration);

        ASSERT_TRUE(std::holds_alternative<double>(tConstraintTargets));
        const auto& tResultTarget = std::get<double>(tConstraintTargets);
        EXPECT_EQ(tResultTarget, 42.0);
    }
    // Vector
    {
        const auto& tVectorConfiguration = tAppConfigurations.front().mConfiguration.mCriteria.back();
        const auto tConstraintTargets =
            detail::make_constraint_target_value(tVectorConstraintInput, tVectorConfiguration);

        ASSERT_TRUE(std::holds_alternative<std::vector<double>>(tConstraintTargets));
        const auto& tResultTarget = std::get<std::vector<double>>(tConstraintTargets);

        ASSERT_EQ(tResultTarget.size(), 2U);
        EXPECT_EQ(tResultTarget.front(), 42.0);
        EXPECT_EQ(tResultTarget.back(), 43.0);
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

TEST(ConstraintTarget, MakeConstraintVectorTarget)
{
    const auto tTargets = std::vector<std::pair<std::string, double>>{{"brick", 91.0}, {"mortar", 90.0}};
    const auto tComponentNameIndices =
        std::map<std::size_t, std::string>{{0U, "gravel"}, {1U, "mortar"}, {2U, "asphalt"}, {3U, "brick"}};
    const auto tConstraintTarget = detail::make_constraint_vector_target(tTargets, tComponentNameIndices);

    EXPECT_EQ(tConstraintTarget.size(), tTargets.size());
    const auto tExpected = std::vector{90.0, 91.0};
    EXPECT_EQ(tConstraintTarget, tExpected);
}

}  // namespace plato::criteria::library::unittest
