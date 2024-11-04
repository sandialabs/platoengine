#include <gtest/gtest.h>

#include <ROL_ParameterList.hpp>
#include <Teuchos_VerbosityLevel.hpp>
#include <filesystem>
#include <variant>
#include <vector>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/rol/OptimizationParameters.hpp"
#include "plato/third_party_integration/rol/ParameterBuilder.hpp"

namespace plato::third_party_integration::rol::unittest
{

namespace
{

template <typename Type>
Type get_parameter(const ROL::ParameterList& aParlist, const ParameterAndValue<Type>& aParameterAndValue)
{
    auto tParlist = aParlist;
    auto tCurrentSublist = detail::retrieve_sublist_from_sequence(tParlist, aParameterAndValue.mSublistNames);
    assert(tCurrentSublist.get().isParameter(aParameterAndValue.mParameterName));
    return tCurrentSublist.get().template get<Type>(aParameterAndValue.mParameterName);
}

using MultiParameterFields = std::variant<ParameterAndValue<bool>,
                                          ParameterAndValue<int>,
                                          ParameterAndValue<double>,
                                          ParameterAndValue<std::string>,
                                          ParameterAndValue<Teuchos::EVerbosityLevel>>;

void compare_field_result_to_gold_list(const ROL::ParameterList& aResult,
                                       const std::vector<MultiParameterFields>& aGoldFields,
                                       const test_utilities::TestContext& aContext)
{
    for (const auto& tGoldField : aGoldFields)
    {
        std::visit(
            [&](const auto& tGoldParameter)
            {
                const auto tResult = get_parameter(aResult, tGoldParameter);
                EXPECT_EQ(tResult, tGoldParameter.mValue) << aContext;
            },
            tGoldField);
    }
}

void test_verbose_parameters(const ROL::ParameterList& aParameterList,
                             const std::pair<int, int> aGold,
                             const test_utilities::TestContext& aTestContext)
{
    const auto tGeneral = get_parameter(aParameterList, make_general_output_level());
    EXPECT_EQ(tGeneral, aGold.first) << aTestContext;
    const auto tComposite = get_parameter(aParameterList, make_step_composite_step_output_level());
    EXPECT_EQ(tComposite, aGold.second) << aTestContext;
}

void test_approximate_hessians(const ROL::ParameterList& aParameterList,
                               const bool aGold,
                               const test_utilities::TestContext& aTestContext)
{
    const auto tGeneral = get_parameter(aParameterList, make_secant_use_as_hessian());
    EXPECT_EQ(tGeneral, aGold) << aTestContext;
    const auto tInexact = get_parameter(aParameterList, make_general_inexact_hessian());
    EXPECT_EQ(tInexact, aGold) << aTestContext;
}

template <typename MemberFunction, typename MakeFunction, typename T>
void test_generic_set_and_member_function(const MemberFunction& aMemberFunction,
                                          const MakeFunction& aMakeFunction,
                                          const T aGoldSetValue,
                                          const test_utilities::TestContext aTestContext)
{
    const auto tDefault = aMakeFunction(std::nullopt);
    const auto tSetValue = aMakeFunction(aGoldSetValue);

    {
        auto tParameterList = OptimizationParameters().parameters();
        compare_field_result_to_gold_list(tParameterList, std::vector<MultiParameterFields>{tDefault},
                                          EXTEND_CONTEXT("Default before set_parameter ", aTestContext));
        detail::set_parameter(tParameterList, tSetValue);
        compare_field_result_to_gold_list(tParameterList, std::vector<MultiParameterFields>{tSetValue},
                                          EXTEND_CONTEXT("Value after set_parameter ", aTestContext));
    }
    {
        auto tParameter = OptimizationParameters();
        compare_field_result_to_gold_list(tParameter.parameters(), std::vector<MultiParameterFields>{tDefault},
                                          EXTEND_CONTEXT("Default before member function setter ", aTestContext));
        aMemberFunction(tParameter, tSetValue.mValue);

        compare_field_result_to_gold_list(tParameter.parameters(), std::vector<MultiParameterFields>{tSetValue},
                                          EXTEND_CONTEXT("Value after member function setter ", aTestContext));
    }
}

}  // namespace

TEST(ROLOptimizationParameters, CreateDefaultWriteAndLoad)
{
    constexpr std::string_view tFileName = "junk.xml";
    auto tParametersDefault = OptimizationParameters();
    tParametersDefault.writeParameters(tFileName);
    const auto tParametersRead = OptimizationParameters(tFileName);

    const auto tGradient = make_status_test_gradient_tolerance();
    const auto tStep = make_status_test_step_tolerance();
    const auto tIteration = make_status_test_iteration_limit();
    const auto tRadius = make_trust_region_initial_radius();
    const auto tVerbose = make_general_output_level();
    const auto tHessian = make_general_inexact_hessian();
    const std::vector<MultiParameterFields> tGoldList{tStep, tGradient, tIteration, tRadius, tVerbose, tHessian};

    compare_field_result_to_gold_list(tParametersRead.parameters(), tGoldList, TEST_CONTEXT("Default file read"));

    std::filesystem::remove(tFileName);
}

TEST(ROLOptimizationParameters, MakeVerbose)
{
    const auto tDefault =
        std::make_pair(make_general_output_level().mValue, make_step_composite_step_output_level().mValue);
    constexpr int tVerbosityExtreme = 4;
    const auto tSetValue = std::make_pair(tVerbosityExtreme, tVerbosityExtreme);

    {
        auto tParameterList = OptimizationParameters().parameters();
        test_verbose_parameters(tParameterList, tDefault,
                                TEST_CONTEXT("Verbose parameter default using detail function"));
        detail::make_verbose(tParameterList);
        test_verbose_parameters(tParameterList, tSetValue, TEST_CONTEXT("Verbose parameter set using detail function"));
    }
    {
        auto tParameter = OptimizationParameters();
        test_verbose_parameters(tParameter.parameters(), tDefault,
                                TEST_CONTEXT("Verbose parameters default using member function"));
        tParameter.verbose();
        test_verbose_parameters(tParameter.parameters(), tSetValue,
                                TEST_CONTEXT("Verbose parameters set using member function"));
    }
}

TEST(ROLOptimizationParameters, ApproximateHessian)
{
    const bool tDefault = make_secant_use_as_hessian().mValue;
    constexpr bool tSetValue = true;
    {
        auto tParameterList = OptimizationParameters().parameters();
        test_approximate_hessians(tParameterList, tDefault,
                                  TEST_CONTEXT("Approximate hessian default using detail function"));
        detail::approximate_hessian(tParameterList);
        test_approximate_hessians(tParameterList, tSetValue,
                                  TEST_CONTEXT("Approximate hessian set using detail function"));
    }
    {
        auto tParameter = OptimizationParameters();
        test_approximate_hessians(tParameter.parameters(), tDefault,
                                  TEST_CONTEXT("Approximate hessian default using member function"));
        tParameter.approximateHessian();
        test_approximate_hessians(tParameter.parameters(), tSetValue,
                                  TEST_CONTEXT("Approximate hessian set using member function"));
    }
}

TEST(ROLOptimizationParameters, GradientTolerance)
{
    test_generic_set_and_member_function(
        [](OptimizationParameters& aParameter, const double aSetValue) { aParameter.gradientTolerance(aSetValue); },
        [](const std::optional<double> aValue) { return make_status_test_gradient_tolerance(aValue); }, 1e-3,
        TEST_CONTEXT("Gradient Tolerance"));
}

TEST(ROLOptimizationParameters, StepTolerance)
{
    test_generic_set_and_member_function(
        [](OptimizationParameters& aParameter, const double aSetValue) { aParameter.stepTolerance(aSetValue); },
        [](const std::optional<double> aValue) { return make_status_test_step_tolerance(aValue); }, 1e-2,
        TEST_CONTEXT("Step Tolerance"));
}

TEST(ROLOptimizationParameters, InitialSearchRadius)
{
    test_generic_set_and_member_function(
        [](OptimizationParameters& aParameter, const double aSetValue) { aParameter.initialSearchRadius(aSetValue); },
        [](const std::optional<double> aValue) { return make_trust_region_initial_radius(aValue); }, 13,
        TEST_CONTEXT("Initial search radius"));
}

TEST(ROLOptimizationParameters, MaximumIterations)
{
    test_generic_set_and_member_function([](OptimizationParameters& aParameter, const int aSetValue)
                                         { aParameter.maximumIterations(static_cast<unsigned>(aSetValue)); },
                                         [](const std::optional<int> aValue)
                                         { return make_status_test_iteration_limit(aValue); },
                                         13, TEST_CONTEXT("Iteration limit"));
}
}  // namespace plato::third_party_integration::rol::unittest
