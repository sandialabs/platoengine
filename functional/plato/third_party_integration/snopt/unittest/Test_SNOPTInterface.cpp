#include <gtest/gtest.h>

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
    return core::make_function([tRosenbrock = aRosenbrock](const linear_algebra::DynamicVector<double>& x)
                               { return tRosenbrock.f(x[0], x[1]); },
                               [tRosenbrock = aRosenbrock](const linear_algebra::DynamicVector<double>& x)
                               { return to_dynamic_vector(tRosenbrock.df(x[0], x[1])); });
}

[[nodiscard]] auto linear_constraint_function()
{
    return core::make_function([](const linear_algebra::DynamicVector<double>& aX) { return aX[0] - aX[1]; },
                               [](const linear_algebra::DynamicVector<double>&) {
                                   return linear_algebra::DynamicVector<double>{1.0, -1.0};
                               });
}

[[nodiscard]] auto nonlinear_constraint_function()
{
    return core::make_function([](const linear_algebra::DynamicVector<double>& aX) { return aX[1] - aX[0] * aX[0]; },
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
    const auto tSolution =
        run_snopt_problem(aInitialGuess, aBounds, std::move(aObjective), std::move(aConstraints), kLogFilePath);

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

}  // namespace plato::third_party_integration::snopt::unittest
