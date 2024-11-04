#include <gtest/gtest.h>

#include <functional>
#include <optional>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/rol/ParameterBuilder.hpp"

namespace plato::third_party_integration::rol::unittest
{

namespace
{

template <typename T>
void test_make_parameter_function(const std::function<ParameterAndValue<T>(const std::optional<T>)>& aFunction,
                                  const std::vector<std::string> aGoldSublistNames,
                                  const std::string& aGoldParameterName,
                                  const T aGoldDefaultValue,
                                  const test_utilities::TestContext& aTestContext)
{
    {
        const auto tResult = aFunction(std::nullopt);
        EXPECT_EQ(tResult.mParameterName, aGoldParameterName) << aTestContext;
        EXPECT_EQ(tResult.mSublistNames, aGoldSublistNames) << aTestContext;
        EXPECT_EQ(tResult.mValue, aGoldDefaultValue) << aTestContext;
    }
    {
        const T tGold{};
        const auto tResult = aFunction(tGold);
        EXPECT_EQ(tResult.mValue, tGold) << aTestContext;
    }
}

}  // namespace

TEST(ParameterBuilder, MakeStatusTestIterationLimit)
{
    test_make_parameter_function<int>([](const std::optional<int> aValue)
                                      { return make_status_test_iteration_limit(aValue); },
                                      {"Status Test"}, "Iteration Limit", 1, TEST_CONTEXT("Iteration limit"));
}

TEST(ParameterBuilder, MakeStatusTestGradientTolerance)
{
    test_make_parameter_function<double>(
        [](const std::optional<double> aValue) { return make_status_test_gradient_tolerance(aValue); }, {"Status Test"},
        "Gradient Tolerance", 1e-12, TEST_CONTEXT("Gradient Tolerance"));
}

TEST(ParameterBuilder, MakeStatusTestStepTolerance)
{
    test_make_parameter_function<double>([](const std::optional<double> aValue)
                                         { return make_status_test_step_tolerance(aValue); },
                                         {"Status Test"}, "Step Tolerance", 1e-14, TEST_CONTEXT("Step Tolerance"));
}

TEST(ParameterBuilder, MakeGeneralOutputLevel)
{
    const auto tVerboseDefault = 1;
    test_make_parameter_function<int>([](const std::optional<int> aValue) { return make_general_output_level(aValue); },
                                      {"General"}, "Output Level", tVerboseDefault, TEST_CONTEXT("General Output"));
}

TEST(ParameterBuilder, MakeStepCompositeStepOutputLevel)
{
    const auto tVerboseDefault = 0;
    test_make_parameter_function<int>(
        [](const std::optional<int> aValue) { return make_step_composite_step_output_level(aValue); },
        {"Step", "Composite Step"}, "Output Level", tVerboseDefault, TEST_CONTEXT("Composite step output"));
}

TEST(ParameterBuilder, MakeTrustRegionInitialRadius)
{
    test_make_parameter_function<double>(
        [](const std::optional<double> aValue) { return make_trust_region_initial_radius(aValue); },
        {"Step", "Trust Region"}, "Initial Radius", 15, TEST_CONTEXT("Trust region radius"));
}

TEST(ParameterBuilder, MakeGeneralInexactHessian)
{
    test_make_parameter_function<bool>(
        [](const std::optional<bool> aValue) { return make_general_inexact_hessian(aValue); }, {"General"},
        "Inexact Hessian-Times-A-Vector", false, TEST_CONTEXT("Inexact Hessian times a vector"));
}

TEST(ParameterBuilder, MakeSecandUseAsHessian)
{
    test_make_parameter_function<bool>(
        [](const std::optional<bool> aValue) { return make_secant_use_as_hessian(aValue); }, {"General", "Secant"},
        "Use as Hessian", false, TEST_CONTEXT("Secant use as Hessian"));
}

}  // namespace plato::third_party_integration::rol::unittest
