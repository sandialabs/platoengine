#include <gtest/gtest.h>

#include "plato/core/Function.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_parser/InputDefinitions.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/services/AppConfiguration.hpp"

namespace plato::criteria::library::unittest
{
namespace
{
[[nodiscard]] auto make_test_criterion_function() -> plato::criteria::library::CriterionFunction
{
    return core::make_function([](const core::MeshProxy&) { return 0.0; },
                               [](const core::MeshProxy&) {
                                   return linear_algebra::DynamicVector<double>{1.0, 2.0};
                               });
}

[[maybe_unused]] static auto kTestCriterionRegistration = plato::criteria::library::CriterionRegistration{
    "test", [](const plato::criteria::library::CriterionInput&) { return make_test_criterion_function(); }};

}  // namespace

TEST(CriterionRegistration, PhonyCriterion)
{
    EXPECT_TRUE(plato::criteria::library::is_criterion_function_registered("test"));
}

TEST(CriterionRegistration, RegistrationNameConfiguration)
{
    constexpr auto tTestAppName = std::string_view{"moose"};
    constexpr auto tTestCriterionName = std::string_view{"squirrel"};
    const auto tCriterionConfiguration = services::CriterionConfiguration{
        /*.mName*/ std::string{tTestCriterionName}, /*.mIsParallelized*/ false, /*.mFunctionName*/ "fun"};
    const auto tAppConfiguration = services::AppConfiguration{/*.mName=*/std::string{tTestAppName},
                                                              /*.mLibraryFileName=*/"lib.so",
                                                              /*.mCriteria=*/{tCriterionConfiguration}};

    const auto tCriterionName = input_parser::CriterionName{std::string{tTestCriterionName}};
    const auto tAppName = input_parser::AppName{std::string{tTestAppName}};

    // Test that we get the same results for each overload
    {
        EXPECT_EQ(criterion_registration_name(tAppConfiguration, tCriterionConfiguration),
                  criterion_registration_name(tAppName, tCriterionName));
    }
    {
        // No app name
        EXPECT_EQ(builtin_criterion_registration_name(tTestCriterionName),
                  criterion_registration_name(boost::none, tCriterionName));
    }
}

TEST(CriterionRegistration, BuiltinRegistrationName)
{
    constexpr auto tTestCriterionName = std::string_view{"cow"};
    const auto tBuiltinRegistrationName = builtin_criterion_registration_name(tTestCriterionName);
    const auto tExpected = std::string{input_parser::kBuiltinAppName} + ":" + std::string{tTestCriterionName};
    EXPECT_EQ(tBuiltinRegistrationName, tExpected);
}

}  // namespace plato::criteria::library::unittest
