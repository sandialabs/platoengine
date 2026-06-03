#include <gtest/gtest.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <map>
#include <set>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/test_utilities/TestAppConfigurations.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/services/AppConfigurationUtilities.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::criteria::library::unittest
{
namespace
{
constexpr auto kArbitraryScalarConstraintTarget = 42.0;
constexpr auto kArbitraryAsiaConstraintTarget = 43.0;
constexpr auto kArbitraryAmericaConstraintTarget = 42.0;

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
    tScalarConstraintInput.constraint_value = kArbitraryScalarConstraintTarget;

    auto tVectorConstraintInput = tBaseConstraint;
    tVectorConstraintInput.app = input_parser::AppName{std::string{tTestAppName}};
    tVectorConstraintInput.criterion = input_parser::CriterionName{"vector-with-components"};
    tVectorConstraintInput.constraint_value_list =
        ConstraintValueList{{input_parser::ComponentAndTarget{/*.component=*/input_parser::IdentifierString{"asia"},
                                                              /*.target=*/kArbitraryAsiaConstraintTarget},
                             input_parser::ComponentAndTarget{/*.component=*/input_parser::IdentifierString{"america"},
                                                              /*.target=*/kArbitraryAmericaConstraintTarget}}};
    return std::make_tuple(std::move(tScalarConstraintInput), std::move(tVectorConstraintInput), tAppConfigurations);
}

[[nodiscard]] auto configuration_with_name(const std::string_view aName,
                                           const std::vector<services::CriterionConfiguration>& aConfigurations)
{
    const auto& tConfigurationWithName = std::ranges::find_if(
        aConfigurations, [aName](const auto& aConfiguration) { return aConfiguration.mName == aName; });
    assert(tConfigurationWithName != aConfigurations.end());
    return *tConfigurationWithName;
}

}  // namespace

TEST(ConstraintTarget, MakeConstraintTargetValue)
{
    const auto [tScalarConstraintInput, tVectorConstraintInput, tAppConfigurations] = test_constraint_inputs();
    {
        const auto tTestContext = TEST_CONTEXT("Scalar");

        const auto& tScalarConfiguration =
            configuration_with_name("scalar", tAppConfigurations.front().mConfiguration.mCriteria);
        const auto tConstraintTargets =
            detail::make_constraint_target_value(tScalarConstraintInput, tScalarConfiguration);

        ASSERT_TRUE(std::holds_alternative<double>(tConstraintTargets)) << tTestContext;
        const auto& tResultTarget = std::get<double>(tConstraintTargets);
        EXPECT_EQ(tResultTarget, kArbitraryScalarConstraintTarget) << tTestContext;
    }
    {
        const auto tTestContext = TEST_CONTEXT("Vector");

        const auto& tVectorConfiguration =
            configuration_with_name("vector-with-components", tAppConfigurations.front().mConfiguration.mCriteria);
        const auto tConstraintTargets =
            detail::make_constraint_target_value(tVectorConstraintInput, tVectorConfiguration);

        ASSERT_TRUE(std::holds_alternative<std::vector<double>>(tConstraintTargets)) << tTestContext;
        const auto& tResultTarget = std::get<std::vector<double>>(tConstraintTargets);

        ASSERT_EQ(tResultTarget.size(), 2U) << tTestContext;
        EXPECT_EQ(tResultTarget.front(), kArbitraryAmericaConstraintTarget) << tTestContext;
        EXPECT_EQ(tResultTarget.back(), kArbitraryAsiaConstraintTarget) << tTestContext;
    }
}

TEST(ConstraintFactory, ConstraintComponentIndices)
{
    const auto [tScalarConstraintInput, tVectorConstraintInput, tAppConfigurations] = test_constraint_inputs();
    {
        const auto& tScalarConfiguration =
            configuration_with_name("scalar", tAppConfigurations.front().mConfiguration.mCriteria);
        const auto tConstraintComponentIndices =
            detail::constraint_component_indices(tScalarConstraintInput, tScalarConfiguration);
        EXPECT_FALSE(tConstraintComponentIndices);
    }
    {
        const auto& tVectorConfiguration =
            configuration_with_name("vector-with-components", tAppConfigurations.front().mConfiguration.mCriteria);

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
