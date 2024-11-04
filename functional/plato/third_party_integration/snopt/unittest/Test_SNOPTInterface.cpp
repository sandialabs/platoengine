#include <gtest/gtest.h>

#include <fstream>
#include <optional>
#include <snoptProblem.hpp>

#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/Rosenbrock.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"
#include "plato/third_party_integration/snopt/SNOPTInterface.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::third_party_integration::snopt::unittest
{
namespace
{
const auto kInitialGuess = std::vector{1.3, 1.2};
const auto kBounds = std::pair{std::vector{0.0, 0.0}, std::vector{2.0, 2.0}};
const auto kExpected = std::vector{1.0, 1.0};
constexpr auto kConstraintTarget = 0.0;
const auto kLogFilePath = std::filesystem::path{"snopt.log"};

[[nodiscard]] auto to_dynamic_vector(const test_utilities::TwoDVector& aX) -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>{std::vector{aX(0), aX(1)}};
}

[[nodiscard]] auto rosenbrock_dynamic_vector_function(const test_utilities::Rosenbrock& aRosenbrock)
{
    return core::make_function_with_first_derivative(
        [tRosenbrock = aRosenbrock](const linear_algebra::DynamicVector<double>& x)
        { return tRosenbrock.f(x[0], x[1]); },
        [tRosenbrock = aRosenbrock](const linear_algebra::DynamicVector<double>& x)
        { return to_dynamic_vector(tRosenbrock.df(x[0], x[1])); });
}

[[nodiscard]] auto linear_constraint_function()
{
    return core::make_function_with_first_derivative([](const linear_algebra::DynamicVector<double>& aX)
                                                     { return aX[0] - aX[1]; },
                                                     [](const linear_algebra::DynamicVector<double>&) {
                                                         return linear_algebra::DynamicVector<double>{1.0, -1.0};
                                                     });
}

[[nodiscard]] auto affine_linear_constraint_function()
{
    return core::make_function_with_first_derivative([](const linear_algebra::DynamicVector<double>& aX)
                                                     { return 2.0 * aX[0] - aX[1] - 1.0; },
                                                     [](const linear_algebra::DynamicVector<double>&) {
                                                         return linear_algebra::DynamicVector<double>{2.0, -1.0};
                                                     });
}

[[nodiscard]] auto nonlinear_constraint_function()
{
    return core::make_function_with_first_derivative(
        [](const linear_algebra::DynamicVector<double>& aX) { return aX[1] - aX[0] * aX[0]; },
        [](const linear_algebra::DynamicVector<double>& aX) {
            return linear_algebra::DynamicVector<double>{-2.0 * aX[0], 1.0};
        });
}

void check_snopt_problem_solution(const std::vector<double>& aInitialGuess,
                                  const SNOPTBounds& aBounds,
                                  ObjectiveType&& aObjective,
                                  ConstraintVectorType&& aConstraints,
                                  const std::vector<double>& aExpectedSolution,
                                  const test_utilities::TestContext& aTestContext)
{
    const auto tSolution = run_snopt_problem(aInitialGuess, aBounds, std::move(aObjective), std::move(aConstraints),
                                             kLogFilePath, SNOPTOptions{});

    constexpr auto tTolerance = 1e-5;
    ASSERT_EQ(aInitialGuess.size(), aExpectedSolution.size()) << aTestContext;
    ASSERT_EQ(aExpectedSolution.size(), tSolution.size()) << aTestContext;
    for (const auto [tSolutionEntry, tExpectedEntry] : utilities::Zip{tSolution, aExpectedSolution})
    {
        EXPECT_NEAR(tSolutionEntry, tExpectedEntry, tTolerance) << aTestContext;
    }
}

}  // namespace

TEST(SNOPTInterface, RosenbrockUnconstrained)
{
    const auto tInitialGuess = std::vector{1.2, 0.8};
    const auto tBounds =
        std::pair{std::vector{-kSNOPTUnbounded, -kSNOPTUnbounded}, std::vector{kSNOPTUnbounded, kSNOPTUnbounded}};
    const auto tExpected = std::vector{1.0, 1.0};
    check_snopt_problem_solution(tInitialGuess, tBounds,
                                 rosenbrock_dynamic_vector_function(test_utilities::Rosenbrock{}), {}, tExpected,
                                 TEST_CONTEXT("Unconstrained"));
    std::filesystem::remove(kLogFilePath);
}

TEST(SNOPTInterface, RosenbrockSolutionOutsideOfBounds)
{
    const auto tInitialGuess = std::vector{3.0, 3.0};
    const auto tBounds = std::pair{std::vector{2.0, 2.0}, std::vector{4.0, 4.0}};
    const auto tExpected = std::vector{2.0, 4.0};
    check_snopt_problem_solution(tInitialGuess, tBounds,
                                 rosenbrock_dynamic_vector_function(test_utilities::Rosenbrock{}), {}, tExpected,
                                 TEST_CONTEXT("Bound constrained"));
    std::filesystem::remove(kLogFilePath);
}

TEST(SNOPTInterface, RosenbrockLinearlyConstrained)
{
    check_snopt_problem_solution(
        kInitialGuess, kBounds, rosenbrock_dynamic_vector_function(test_utilities::Rosenbrock{}),
        ConstraintVectorType{{linear_constraint_function(), kConstraintTarget, Linearity::kLinear}}, kExpected,
        TEST_CONTEXT("Linearly constrained"));
    std::filesystem::remove(kLogFilePath);
}

TEST(SNOPTInterface, RosenbrockAffineLinearlyConstrained)
{
    check_snopt_problem_solution(
        kInitialGuess, kBounds, rosenbrock_dynamic_vector_function(test_utilities::Rosenbrock{}),
        ConstraintVectorType{{affine_linear_constraint_function(), kConstraintTarget, Linearity::kLinear}}, kExpected,
        TEST_CONTEXT("Affine-linearly constrained"));
    std::filesystem::remove(kLogFilePath);
}

TEST(SNOPTInterface, RosenbrockNonlinearlyConstrained)
{
    check_snopt_problem_solution(
        kInitialGuess, kBounds, rosenbrock_dynamic_vector_function(test_utilities::Rosenbrock{}),
        ConstraintVectorType{{nonlinear_constraint_function(), kConstraintTarget, Linearity::kNonlinear}}, kExpected,
        TEST_CONTEXT("Nonlinearly constrained"));
    std::filesystem::remove(kLogFilePath);
}

TEST(SNOPTInterface, RosenbrockLinearlyAndNonlinearlyConstrained)
{
    check_snopt_problem_solution(kInitialGuess, kBounds,
                                 rosenbrock_dynamic_vector_function(test_utilities::Rosenbrock{}),
                                 ConstraintVectorType{
                                     {linear_constraint_function(), kConstraintTarget, Linearity::kLinear},
                                     {nonlinear_constraint_function(), kConstraintTarget, Linearity::kNonlinear},
                                 },
                                 kExpected, TEST_CONTEXT("Nonlinearly and linearly constrained"));
    std::filesystem::remove(kLogFilePath);
}

TEST(SNOPTInterface, RosenbrockLinearVectorConstraint)
{
    // Test that a linear vector constraint works
    EXPECT_EQ(1, 0);
}

TEST(SNOPTInterface, RosenbrockNoninearVectorConstraint)
{
    // Test that a non-linear vector constraint works
    EXPECT_EQ(1, 0);
}

namespace
{

const auto kSuccessfulGetParameterCode = int{0};

void write_snoptions_file(const std::string& aFileName)
{
    const std::string tFileContent = R"(Begin options
        Time limit 3
        Iteration limit 400
    End options
    )";

    std::ofstream tOutput(aFileName.c_str());
    tOutput << tFileContent;
}

void get_parameter_and_test_value(snoptProblemA& aProblem,
                                  const std::string_view aParameter,
                                  const int aGold,
                                  const test_utilities::TestContext aTestContext)
{
    int tParameter;
    const int tReturnCode = aProblem.getIntParameter(aParameter.data(), tParameter);
    EXPECT_EQ(tReturnCode, kSuccessfulGetParameterCode);
    EXPECT_EQ(tParameter, aGold) << aTestContext;
}

[[nodiscard]] auto create_problem_and_apply_options(const SNOPTOptions& aOptions) -> snoptProblemA
{
    constexpr auto tUseSummaryFile = int{1};
    snoptProblemA tProblem{};
    tProblem.initialize(kLogFilePath.c_str(), tUseSummaryFile);
    detail::apply_options(tProblem, aOptions);
    return tProblem;
}

}  // namespace

TEST(SNOPTInterfaceDetail, ApplyOptions)
{
    const auto tInputFile = std::string{"snoptions.in"};
    constexpr int tPlatoTimeLimit = 42;
    constexpr int tPlatoMaxIterationLimit = 666;
    constexpr int tUninitializedSNOPTParameterValue = -11111;

    SNOPTOptions tOptions{std::nullopt, tPlatoTimeLimit, tPlatoMaxIterationLimit};
    {
        auto tProblem = create_problem_and_apply_options(tOptions);
        get_parameter_and_test_value(tProblem, snopt::kMajorIterationLimitName, tPlatoMaxIterationLimit,
                                     TEST_CONTEXT("plato major iteration spec applied"));
    }

    write_snoptions_file(tInputFile);
    tOptions.mFilePath = tInputFile;
    {
        auto tProblem = create_problem_and_apply_options(tOptions);
        get_parameter_and_test_value(tProblem, snopt::kMajorIterationLimitName, tPlatoMaxIterationLimit,
                                     TEST_CONTEXT("external major iteration input file overridden by plato spec"));
    }

    tOptions.mMajorIterationLimit = std::nullopt;
    tOptions.mTimeLimitInMinutes = std::nullopt;
    {
        auto tProblem = create_problem_and_apply_options(tOptions);
        get_parameter_and_test_value(tProblem, snopt::kMajorIterationLimitName, tUninitializedSNOPTParameterValue,
                                     TEST_CONTEXT("external major iteration input file value"));
    }
    test_utilities::test_for_existence_and_remove({tInputFile, kLogFilePath},
                                                  TEST_CONTEXT("Clean up files SNOPT Interface"));
}

}  // namespace plato::third_party_integration::snopt::unittest
